#ifndef LLL_INTEGER_MATH_HPP
#define LLL_INTEGER_MATH_HPP

#include "../integer.hpp"

namespace lll {
Integer pow(const Integer &a, uint64_t b);
Integer gcd(const Integer &a, const Integer &b);
Integer sqrt(const Integer &n);
bool is_prime(const Integer &n);
}

#endif // LLL_INTEGER_MATH_HPP
