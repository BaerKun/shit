#include "internal.hpp"
#include <stdexcept>

namespace lll {
using namespace internal;

// quotient is always 64-bits.
static inline uint64_t div128(const uint64_t high, const uint64_t low,
                              const uint64_t divisor, uint64_t &rem) {
  return _udiv128(high, low, divisor, &rem);
}

static void div_impl(const VecU64 &a, const VecU64 &b, VecU64 &quot,
                     VecU64 &rem) {
  const size_t size_a = a.size();
  const size_t size_b = b.size();

  if (size_b == 1) {
    const uint64_t divisor = b[0];
    VecU64 quot_tmp(size_a);
    uint64_t high = 0;

    for(size_t i = size_a; i--;) {
      quot_tmp[i] = div128(high, a[i], divisor, high);
    }
    if(quot_tmp[size_a - 1] == 0) quot.pop_back();
    quot = std::move(quot_tmp);
    rem = {high};
    return;
  }

}

void div(const Integer &a, const Integer &b, Integer &quot, Integer &rem) {
  if(b.zero()) throw std::runtime_error("Division by zero");
  if(a.zero()) {
    quot = 0;
    rem = 0;
    return;
  }

  const VecU64 &abs_a = a.abs_val_;
  const VecU64 &abs_b = b.abs_val_;

  div_impl(abs_a, abs_b, quot.abs_val_, rem.abs_val_);

  quot.neg_ = !quot.zero() && a.neg_ ^ b.neg_;
  rem.neg_ = !rem.zero() && a.neg_;
}
} // namespace lll