#include "internal.hpp"

namespace lll {
using namespace internal;

static void grade_school(const VecU64 &a, const VecU64 &b, VecU64 &out) {
  const size_t size_a = a.size();
  const size_t size_b = a.size();
  VecU64 res(size_a + size_b, 0);

  for (size_t i = 0; i < size_a; i++) {
    uint64_t high, low, carry = 0;
    for (size_t j = 0; j < size_b; j++) {
      mul64(a[i], b[j], high, low);
      uint64_t &sum = res[i + j];
      sum += carry;
      const uint64_t carry1 = sum < high;
      sum += low;
      const uint64_t carry2 = sum < low;
      carry = high + carry1 + carry2;
    }
    res[i + size_b] = carry;
  }
  if (res[size_a + size_b - 1] == 0) res.pop_back();

  out = std::move(res);
}

void mul(const Integer &a, const Integer &b, Integer &out) {
  if (a.zero() || b.zero()) {
    out = 0;
    return;
  }

  const VecU64 &abs_a = a.abs_val_;
  const VecU64 &abs_b = b.abs_val_;

  grade_school(abs_a, abs_b, out.abs_val_);

  out.neg_ = a.neg_ ^ b.neg_;
}
} // namespace lll