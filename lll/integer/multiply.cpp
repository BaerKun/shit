#include "internal.hpp"

namespace lll {
using namespace internal;

void internal::umul_64bits_(const VecU64 &a, const uint64_t b, VecU64 &out) {
  const size_t size = a.size();
  uint64_t high, low, carry = 0;

  out.resize(size + 1);
  for (uint64_t i = 0; i < size; i++) {
    mul64(a[i], b, high, low);
    out[i] = add64(low, carry, 0, carry);
    carry += high;
  }
  if (carry) out.back() = carry;
  else out.pop_back();
}

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

void Integer::mul_64bits(const Integer &a, const int64_t b, Integer &out) {
  if (a.zero() || b == 0) {
    out = 0;
    return;
  }
  umul_64bits_(a.abs_val_, std::abs(b), out.abs_val_);
  out.neg_ = a.neg_ ^ (b < 0);
}

void Integer::mul(const Integer &a, const Integer &b, Integer &out) {
  if (a.zero() || b.zero()) {
    out = 0;
    return;
  }

  grade_school(a.abs_val_, b.abs_val_, out.abs_val_);
  out.neg_ = a.neg_ ^ b.neg_;
}
} // namespace lll