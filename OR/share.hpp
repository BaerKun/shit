#ifndef SHARE_HPP
#define SHARE_HPP

#include <Eigen/Core>
#include <limits>

namespace OR {
using DynamicMatrix = Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic>;

enum Optim { Maximize, Minimize };

enum Comparison { LESS_EQUAL = -1, EQUAL = 0, GREATER_EQUAL = 1 };

template <typename T>
constexpr T EPSILON = std::numeric_limits<T>::epsilon() * 100;
} // namespace OR

#endif // SHARE_HPP
