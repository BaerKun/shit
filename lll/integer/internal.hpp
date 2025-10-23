#ifndef LLL_INTEGER_INTERNAL_HPP
#define LLL_INTEGER_INTERNAL_HPP

#include "../integer.hpp"

namespace lll {
using VecU64 = std::vector<uint64_t>;
namespace internal {
static inline uint64_t add64(const uint64_t a, const uint64_t b,
                             uint64_t &carry) {
  const uint64_t sum = a + b + carry;
  carry = sum < a || sum < b;
  return sum;
}

static inline uint64_t sub64(const uint64_t a, const uint64_t b,
                             uint64_t &borrow) {
  uint64_t diff = a - b;
  const uint64_t borrow_ = a < b || diff < borrow;
  diff -= borrow;
  borrow = borrow_;
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
