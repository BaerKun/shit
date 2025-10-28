#include "internal.hpp"

namespace lll {
using namespace internal;

static void grade_school(const VecU64 &a, const VecU64 &b, VecU64 &out) {
  const size_t size_a = a.size();
  const size_t size_b = b.size();
  VecU64 res(size_a + size_b, 0);

  for (size_t i = 0; i < size_a; i++) {
    uint64_t high, low, carry = 0;
    for (size_t j = 0; j < size_b; j++) {
      mul64(a[i], b[j], high, low);
      res[i + j] = add64(res[i + j], low, carry, carry);
      carry += high;
    }
    res[i + size_b] = carry;
  }
  norm_top(res);

  out = std::move(res);
}

void mul(const Integer &a, const Integer &b, Integer &out) {
  if (a.zero() || b.zero()) {
    out = 0;
    return;
  }

  grade_school(a.abs_val, b.abs_val, out.abs_val);

  out.neg = a.neg ^ b.neg;
}
} // namespace lll