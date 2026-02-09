#ifndef LLL_INTEGER_MATH_HPP
#define LLL_INTEGER_MATH_HPP

#include "../integer.hpp"

namespace lll {
void pow(const Integer &a, uint64_t b, Integer &out);
void gcd(const Integer &a, const Integer &b, Integer &out);
void sqrt(const Integer &n, Integer &out);
bool is_prime(const Integer &n);
}

#endif // LLL_INTEGER_MATH_HPP
