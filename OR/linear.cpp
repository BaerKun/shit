#include "linear.hpp"

namespace OR {
bool LinearProgram::solve(Eigen::VectorX<float> &solution, float &minmax) {
  const Eigen::Index numObjVar = objective_.cols();
  const auto numConstr = (Eigen::Index)constraints_.size();

  Eigen::VectorX<Eigen::Index> basic(constraints_.size());
  Eigen::Index numSlackVar = 0; // 松弛变量
  Eigen::Index numArtificialVar = 0; // 人工变量
  for (const auto &constr : constraints_) {
    switch (constr.comp) {
    case EQUAL:
      numArtificialVar += 1;
      break;
    case LESS_EQUAL:
      numSlackVar += 1;
      break;
    case GREATER_EQUAL:
      numSlackVar += 1;
      numArtificialVar += 1;
    default:
      break;
    }
  }
  const Eigen::Index numObjAndSlackVar = numObjVar + numSlackVar;
  const Eigen::Index totalVar = numObjAndSlackVar + numArtificialVar;

  // 初始化标准型矩阵
  tableau_ = SimplexTableau::Zero(numConstr + 1, totalVar + 1);
  auto rhs = tableau_.col(totalVar);

  for (Eigen::Index r = 0, slackIdx = numObjVar,
                    artificialIdx = numObjAndSlackVar;
       r < numConstr; ++r) {
    const auto &constr = constraints_[r];
    auto row = tableau_.row(r);
    row.head(numObjVar) = constr.coef;
    rhs(r) = constr.rhs;

    Eigen::Index entering = -1;
    switch (constr.comp) {
    case EQUAL:
      entering = artificialIdx++;
      break;
    case LESS_EQUAL:
      entering = slackIdx++;
      break;
    case GREATER_EQUAL:
      row(slackIdx++) = -1;
      entering = artificialIdx++;
    default:
      break;
    }
    row(entering) = 1;
    basic(r) = entering;
  }

  Eigen::VectorX<float> var(totalVar);
  auto reducedCost = tableau_.row(numConstr);

  // 直接单纯形法
  if (numArtificialVar == 0) {
    reducedCost.head(numObjVar) = objective_;
    if (!simplexMethodMax(tableau_, basic, var, minmax)) return false;
    solution = var.head(numObjVar);
    return true;
  }

  // 两阶段法
  // 第一阶段：Minimize sum(artificial)
  reducedCost.segment(numObjAndSlackVar, numArtificialVar).setOnes();
  for (Eigen::Index i = 0; i < basic.size(); ++i) {
    if (basic[i] >= numObjAndSlackVar) reducedCost -= tableau_.row(i);
  }

  if (!simplexMethodMin(tableau_, basic, var, minmax)) return false;
  if (minmax >= EPSILON<float>) return false;

  // 第二阶段
  tableau_.col(numObjAndSlackVar) = rhs; // 复制rhs，防止resize后丢失
  tableau_.conservativeResize(
      numConstr + 1, numObjAndSlackVar + 1); // 重新分配内存，并保留部分原有数据

  auto reducedCost2 =
      tableau_.row(numConstr); // Block不能重新引用，必须声明新变量
  reducedCost2.head(numObjVar) = objective_;
  reducedCost2.segment(numObjVar, numSlackVar + 1).setZero();

  for (Eigen::Index i = 0; i < basic.size(); ++i) {
    if (const Eigen::Index b = basic(i);
      b < numObjVar)
      reducedCost2 -= tableau_.row(i) * objective_(b);
  }

  var.resize(numObjAndSlackVar);
  if (!simplexMethodMax(tableau_, basic, var, minmax)) return false;
  solution = var.head(numObjVar);
  return true;
}


template <int Optim>
bool simplexMethod(DynamicMatrix &tableau,
                   Eigen::VectorX<Eigen::Index> &basic,
                   Eigen::VectorX<float> &solution, float &minmax) {
  auto rhs = tableau.col(tableau.cols() - 1).head(basic.rows());
  auto reducedCost = tableau.row(rhs.rows()).head(tableau.cols() - 1); // 检验数

  for (Eigen::Index iter = 0; iter < tableau.rows(); ++iter) {
    // choose max/min objective coefficient (c)
    Eigen::Index entering; // 入基变量
    if (Optim == Maximize
          ? reducedCost.maxCoeff(&entering) <= EPSILON<float>
          : reducedCost.minCoeff(&entering) >= -EPSILON<float>) {
      solution = Eigen::VectorX<float>::Zero(reducedCost.cols());
      for (Eigen::Index r = 0; r < rhs.rows(); ++r) {
        solution(basic(r)) = rhs(r);
      }
      minmax = -tableau(rhs.rows(), reducedCost.cols());
      return true;
    }
    auto col = tableau.col(entering);

    // choose min theta(b/a)
    Eigen::Index leaving = rhs.rows(); // 出基
    float minTheta = std::numeric_limits<float>::max();
    for (Eigen::Index r = 0; r < rhs.rows(); ++r) {
      if (col(r) > 0) {
        if (const float theta = rhs(r) / col(r); theta < minTheta) {
          minTheta = theta;
          leaving = r;
        }
      }
    }
    if (leaving == rhs.rows()) {
      return false;
    }

    // update
    auto row = tableau.row(leaving);
    row /= row(entering);
    for (Eigen::Index r = 0; r < tableau.rows(); ++r) {
      if (r != leaving) {
        tableau.row(r) -= row * col(r);
      }
    }
    basic(leaving) = entering;
  }
  return false;
}

template <int Optim>
bool dualSimplexMethod(DynamicMatrix &tableau,
                       Eigen::VectorX<Eigen::Index> &basic,
                       Eigen::VectorX<float> &solution, float &minmax) {
  auto rhs = tableau.col(tableau.cols() - 1).head(basic.rows());
  auto reducedCost = tableau.row(rhs.rows()).head(tableau.cols() - 1);

  for (Eigen::Index i = 0; i < tableau.rows(); ++i) {
    // choose min rhs(b)
    Eigen::Index leaving;
    if (rhs.minCoeff(&leaving) >= -EPSILON<float>) {
      solution = Eigen::VectorX<float>::Zero(reducedCost.cols());
      for (Eigen::Index r = 0; r < rhs.rows(); ++r) {
        solution(basic(r)) = rhs(r);
      }
      minmax = -tableau(rhs.rows(), reducedCost.cols());
      return true;
    }
    auto row = tableau.row(leaving);

    // choose min/max theta
    Eigen::Index entering = reducedCost.cols();
    if constexpr (Optim == Maximize) {
      float minTheta = std::numeric_limits<float>::max();
      for (Eigen::Index c = 0; c < reducedCost.cols(); ++c) {
        if (row(c) < 0) {
          if (const float theta = reducedCost(c) / row(c); theta < minTheta) {
            minTheta = theta;
            entering = c;
          }
        }
      }
    } else {
      float maxTheta = -std::numeric_limits<float>::max();
      for (Eigen::Index c = 0; c < reducedCost.cols(); ++c) {
        if (row(c) < 0) {
          if (const float theta = reducedCost(c) / row(c); theta > maxTheta) {
            maxTheta = theta;
            entering = c;
          }
        }
      }
    }
    if (entering == reducedCost.cols()) {
      return false;
    }

    // update
    auto col = tableau.col(entering);
    row /= row(entering);
    for (Eigen::Index r = 0; r < tableau.rows(); ++r) {
      if (r != leaving) {
        tableau.row(r) -= row * col(r);
      }
    }
    basic(leaving) = entering;
  }
  return false;
}
} // namespace OR