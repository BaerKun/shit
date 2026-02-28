#ifndef LLL_INTEGER_MATH_HPP
#define LLL_INTEGER_MATH_HPP

#include "../integer.hpp"

namespace lll {
Integer pow(const Integer &b, uint64_t e);
Integer pow_mod(const Integer &b, const Integer &e, const Integer &m);
Integer gcd(const Integer &a, const Integer &b);
Integer sqrt(const Integer &n);
bool is_prime(const Integer &n);
bool prime_test(const Integer &n, size_t t = 1);
}

#endif // LLL_INTEGER_MATH_HPP