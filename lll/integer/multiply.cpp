#include "internal.hpp"

namespace lll {
using namespace internal;

void internal::mul_64bits_internal(const VecU64 &a, const uint64_t b,
                                   VecU64 &out) {
  VecU64 tmp(a.size() + 1);
  uint64_t high, low, carry = 0;
  for (uint64_t i = 0; i < a.size(); i++) {
    mul64(a[i], b, high, low);
    tmp[i] = add64(tmp[i], low, carry, carry);
    carry += high;
  }
  if (carry) tmp.back() = carry;
  else tmp.pop_back();
  out = std::move(tmp);
}

void Integer::mul_64bits(const Integer &a, const int64_t b, Integer &out) {
  if (a.zero() || b == 0) {
    out = 0;
    return;
  }
  mul_64bits_internal(a.abs_val_, std::abs(b), out.abs_val_);
  out.neg_ = a.neg_ ^ (b < 0);
}


static void grade_school(const VecU64 &a, const VecU64 &b, VecU64 &out) {
  const size_t size_a = a.size();
  const size_t size_b = b.size();
  VecU64 tmp(size_a + size_b, 0);

  for (size_t i = 0; i < size_a; i++) {
    uint64_t high, low, carry = 0;
    for (size_t j = 0; j < size_b; j++) {
      mul64(a[i], b[j], high, low);
      tmp[i + j] = add64(tmp[i + j], low, carry, carry);
      carry += high;
    }
    tmp[i + size_b] = carry;
  }
  norm_top(tmp);
  out = std::move(tmp);
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