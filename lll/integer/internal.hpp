#ifndef LLL_INTEGER_INTERNAL_HPP
#define LLL_INTEGER_INTERNAL_HPP

#include "../integer.hpp"

namespace lll {
using VecU64 = std::vector<uint64_t>;
namespace internal {
// a + b + c, carry = 0, 1, 2
static inline uint64_t add64(const uint64_t a, const uint64_t b, const uint64_t c,
                             uint64_t &carry) {
  uint64_t sum = a + b;
  const uint64_t carry1 = sum < b;

  sum += c;
  const uint64_t carry2 = sum < c;

  carry = carry1 + carry2;
  return sum;
}

// a - b - c, borrow = 0, 1, 2
static inline uint64_t sub64(const uint64_t a, const uint64_t b, const uint64_t c,
                             uint64_t &borrow) {
  uint64_t diff = a - b;
  const uint64_t borrow1 = a < b;

  diff -= c;
  const uint64_t borrow2 = diff < c;

  borrow = borrow1 + borrow2;
  return diff;
}

static inline void mul64(const uint64_t a, const uint64_t b, uint64_t &high,
                         uint64_t &low) {
#if defined(_MSC_VER)
  low = _umul128(a, b, &high);
#elif defined(__GNUC__) || defined(__clang__)
  const __uint128_t prod = (__uint128_t)a * (__uint128_t)b;
  low = (uint64_t)prod;
  high = (uint64_t)(prod >> 64);
#endif
}

int ucmpr(const VecU64 &a, const VecU64 &b);
void uadd(const VecU64 &max, const VecU64 &min, VecU64 &out);
void usub(const VecU64 &max, const VecU64 &min, VecU64 &out);
} // namespace internal
} // namespace lll

#endif // LLL_INTEGER_INTERNAL_HPP
