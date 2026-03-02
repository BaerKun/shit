#pragma once

#include "transport.hpp"
#include <limits>
#include <vector>

namespace OR {
// 二维循环链表，一定程度可以视作二叉树
struct BasicVariable {
  Eigen::Index rowIdx, colIdx;
  BasicVariable *next = nullptr;
  BasicVariable *down = nullptr;

  explicit BasicVariable(const Eigen::Index r = -1, const Eigen::Index c = -1)
    : rowIdx(r), colIdx(c) {
  }
};

class BasicContainer {
public:
  BasicContainer(const Eigen::Index rows, const Eigen::Index cols)
    : row_(rows, nullptr), col_(cols, nullptr) {
  }

  [[nodiscard]] BasicVariable *getHead() const { return row_[0]; }

  void push(BasicVariable *const node) {
    if (BasicVariable *&left = row_[node->rowIdx]; left == nullptr) {
      left = node->next = node;
    } else {
      node->next = left->next;
      left->next = node;
    }
    if (BasicVariable *&top = col_[node->colIdx]; top == nullptr) {
      top = node->down = node;
    } else {
      node->down = top->down;
      top->down = node;
    }
  }

  void unlink(const BasicVariable *const node) {
    auto left = row_[node->rowIdx];
    if (node == left) row_[node->rowIdx] = node->next;
    for (; left->next != node; left = left->next);
    left->next = node->next;

    auto top = col_[node->colIdx];
    if (node == top) col_[node->colIdx] = node->down;
    for (; top->down != node; top = top->down);
    top->down = node->down;
  }

  float finalWork(const DynamicMatrix &cost,
                  const DynamicMatrix &solution) const {
    float maxMin = 0;
    for (const BasicVariable *head : row_) {
      const BasicVariable *node = head;
      do {
        const auto next = node->next;
        maxMin += cost(node->rowIdx, node->colIdx) *
            solution(node->rowIdx, node->colIdx);
        delete node;
        node = next;
      } while (node != head);
    }
    return maxMin;
  }

private:
  std::vector<BasicVariable *> row_;
  std::vector<BasicVariable *> col_;
};

void northwestCorner(Eigen::VectorX<float> &supply,
                     Eigen::RowVectorX<float> &demand,
                     DynamicMatrix &solution, BasicContainer &basic) {
  Eigen::Index r = 0, c = 0;
  while (true) {
    basic.push(new BasicVariable(r, c));
    if (supply[r] < demand[c]) {
      solution(r, c) = supply[r];
      demand[c] -= supply[r];
      supply[r] = 0;
      if (++r == supply.rows()) break;
    } else {
      solution(r, c) = demand[c];
      supply[r] -= demand[c];
      demand[c] = 0;
      if (++c == demand.cols()) break;
    }
  }
}

class ReducedCost {
public:
  explicit ReducedCost(DynamicMatrix &cost)
    : cost_(cost), reducedCost_(cost.rows(), cost.cols()), u(cost.rows()),
      v(cost.cols()) {
    entering_ = new BasicVariable;
  }

  ~ReducedCost() { delete entering_; }

  void init(const BasicVariable *const head) {
    u(head->rowIdx) = 0;
    v(head->colIdx) = cost_(head->rowIdx, head->colIdx);
    recursionStep(head, head, head);
    refresh();
  }

  BasicVariable *findEntering() const {
    Eigen::Index enteringRow, enteringCol;
    if (const float minReducedCost = reducedCost_.
        minCoeff(&enteringRow, &enteringCol); minReducedCost >= -EPSILON<float>)
      return nullptr;

    entering_->rowIdx = enteringRow;
    entering_->colIdx = enteringCol;
    return entering_;
  }

  void update(BasicContainer &basic, BasicVariable *const leaving) {
    basic.unlink(leaving);
    u(entering_->rowIdx) =
        cost_(entering_->rowIdx, entering_->colIdx) - v(entering_->colIdx);
    recursionStep(entering_, entering_, entering_->down);

    refresh();
    entering_ = leaving; // 内存复用
  }

private:
  void refresh() {
    for (Eigen::Index r = 0; r < cost_.rows(); ++r) {
      for (Eigen::Index c = 0; c < cost_.cols(); ++
           c)
        reducedCost_(r, c) = cost_(r, c) - u(r) - v(c);
    }
  }

  void recursionStep(const BasicVariable *const node,
                     const BasicVariable *const rowHead,
                     const BasicVariable *const colHead) {
    if (const auto next = node->next; next != rowHead) {
      v(next->colIdx) = cost_(next->rowIdx, next->colIdx) - u(next->rowIdx);
      recursionStep(next, rowHead, next);
    }
    if (const auto down = node->down; down != colHead) {
      u(down->rowIdx) = cost_(down->rowIdx, down->colIdx) - v(down->colIdx);
      recursionStep(down, down, colHead);
    }
  }

  DynamicMatrix &cost_;
  DynamicMatrix reducedCost_;
  Eigen::VectorX<float> u;
  Eigen::RowVectorX<float> v;
  BasicVariable *entering_;
};

class ClosedLoop {
public:
  explicit ClosedLoop(const size_t capacity) { closedLoop_.reserve(capacity); }

  void find(BasicContainer &basic, BasicVariable *entering) {
    basic.push(entering);
    entering_ = entering;
    closedLoop_.clear();
    recursionStep(entering);
  }

  BasicVariable *update(DynamicMatrix &solution) const {
    float adjustment = std::numeric_limits<float>::max();
    BasicVariable *leaving = nullptr;
    for (size_t i = 1; i < closedLoop_.size(); i += 2) {
      if (const auto node = closedLoop_[i];
        solution(node->rowIdx, node->colIdx) < adjustment) {
        adjustment = solution(node->rowIdx, node->colIdx);
        leaving = closedLoop_[i];
      }
    }
    for (size_t i = 0; i < closedLoop_.size(); i += 2) {
      const auto plus = closedLoop_[i];
      solution(plus->rowIdx, plus->colIdx) += adjustment;

      const auto minus = closedLoop_[i + 1];
      solution(minus->rowIdx, minus->colIdx) -= adjustment;
    }
    return leaving;
  }

private:
  bool recursionStep(BasicVariable *const node) {
    closedLoop_.push_back(node);
    for (auto nodeRow = node->next; nodeRow != node; nodeRow = nodeRow->next) {
      closedLoop_.push_back(nodeRow);
      for (auto nodeCol = nodeRow->down; nodeCol != nodeRow;
           nodeCol = nodeCol->down) {
        if (nodeCol == entering_ || recursionStep(nodeCol)) return true;
      }
      closedLoop_.pop_back();
    }
    closedLoop_.pop_back();
    return false;
  }

  std::vector<BasicVariable *> closedLoop_;
  BasicVariable *entering_ = nullptr;
};

void closedLoopAdjust(DynamicMatrix &cost, BasicContainer &basic,
                      DynamicMatrix &solution) {
  ClosedLoop closedLoop(cost.rows() + cost.cols());
  ReducedCost reducedCost(cost);

  reducedCost.init(basic.getHead());
  while (true) {
    const auto entering = reducedCost.findEntering();
    if (entering == nullptr) return;
    closedLoop.find(basic, entering);

    const auto leaving = closedLoop.update(solution);
    reducedCost.update(basic, leaving);
  }
}

void transportSimplexMethodMax(DynamicMatrix &cost,
                               Eigen::VectorX<float> &supply,
                               Eigen::RowVectorX<float> &demand,
                               DynamicMatrix &solution, float &minmax) {
  BasicContainer basic(cost.rows(), cost.cols());
  solution = DynamicMatrix::Zero(cost.rows(), cost.cols());

  northwestCorner(supply, demand, solution, basic);
  closedLoopAdjust(cost, basic, solution);

  minmax = basic.finalWork(cost, solution);
}
} // namespace OR