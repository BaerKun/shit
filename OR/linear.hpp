#ifndef LINEAR_HPP
#define LINEAR_HPP

#include "share.hpp"
#include <Eigen/Core>
#include <vector>

namespace OR {
struct Constraint {
  Eigen::RowVectorX<float> coef{}; // coefficients 系数
  int comp{};
  float rhs; // right-hand Side 即 b
};

/*
 * only Max
 */
class LinearProgram {
public:
  using SimplexTableau = DynamicMatrix;

  LinearProgram() = default;

  void setObjective(const Eigen::RowVectorX<float> &objective) {
    objective_ = objective;
  }

  void setObjective(Eigen::RowVectorX<float> &&objective) {
    objective_ = std::move(objective);
  }

  void addConstraint(const Constraint &constraint) {
    if (constraint.rhs >= 0) constraints_.push_back(constraint);
    else
      constraints_.push_back(
          {-constraint.coef, -constraint.comp, -constraint.rhs});
  }

  void addConstraint(Constraint &&constraint) {
    if (constraint.rhs < 0) {
      constraint.coef = -constraint.coef;
      constraint.comp = -constraint.comp;
      constraint.rhs = -constraint.rhs;
    }
    constraints_.push_back(std::move(constraint));
  }

  [[nodiscard]] const SimplexTableau &getSimplexTableau() const {
    return tableau_;
  }

  bool solve(Eigen::VectorX<float> &solution, float &minmax);

private:
  Eigen::RowVectorX<float> objective_{};
  std::vector<Constraint> constraints_{};
  SimplexTableau tableau_{};
};

/*
 * tableau = [ A  b
 *             c -Z ]
 * b >= 0
 */
bool simplexMethodMax(DynamicMatrix &tableau,
                      Eigen::VectorX<Eigen::Index> &basic,
                      Eigen::VectorX<float> &solution, float &minmax);

bool simplexMethodMin(DynamicMatrix &tableau,
                      Eigen::VectorX<Eigen::Index> &basic,
                      Eigen::VectorX<float> &solution, float &minmax);

/*
 * tableau = [ A  b
 *             c -Z ]
 * c <= 0 (Max) or c >= 0 (Min)
 */
bool dualSimplexMethod(DynamicMatrix &tableau,
                       Eigen::VectorX<Eigen::Index> &basic,
                       Eigen::VectorX<float> &solution, float &minmax);
} // namespace OR

#endif // LINEAR_HPP