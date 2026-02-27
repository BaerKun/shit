#include "internal.hpp"

namespace lll {
using namespace internal;

void shl_abs_(const VecU64 &a, const uint64_t b, VecU64 &out) {
  if (a.empty()) {
    out.clear();
    return;
  }
  if (b == 0) {
    out = a;
    return;
  }

  const size_t size_a = a.size();
  const size_t shift_limb = b / 64;
  const size_t shift_bit = b % 64;

  // TODO: fix a == out
  if (shift_bit == 0) {
    const size_t size_o = size_a + shift_limb;
    out.assign(size_o, 0);
    for (size_t i = 0; i < size_a; i++) {
      out[i + shift_limb] = a[i];
    }
    return;
  }

  const size_t size_o = size_a + shift_limb + 1;
  out.assign(size_o, 0);

  uint64_t carry = 0;
  for (size_t i = 0; i < size_a; i++) {
    const uint64_t curr = a[i];
    out[i + shift_limb] = curr << shift_bit | carry;
    carry = curr >> (64 - shift_bit);
  }
  if (carry) {
    out[size_a + shift_limb] = carry;
  } else {
    out.pop_back();
  }
}

void shr_abs_(const VecU64 &a, const uint64_t b, VecU64 &out) {
  if (a.empty() || a.size() <= b / 64) {
    out.clear();
    return;
  }
  if (b == 0) {
    out = a;
    return;
  }

  const size_t size_a = a.size();
  const size_t shift_limb = b / 64;
  const size_t shift_bit = b % 64;
  const size_t size_o = size_a - shift_limb;
  out.resize(size_o);

  if (shift_bit == 0) {
    for (size_t i = 0; i < size_o; i++) {
      out[i] = a[i + shift_limb];
    }
    return;
  }

  uint64_t carry = 0;
  for (size_t i = size_o; i--;) {
    const uint64_t curr = a[i + shift_limb];
    out[i] = curr >> shift_bit | carry;
    carry = curr << (64 - shift_bit);
  }
  norm_top(out);
}

void Integer::shl_abs(const Integer &a, const uint64_t b, Integer &out) {
  shl_abs_(a.abs_val_, b, out.abs_val_);
  out.neg_ = a.neg_;
}

void Integer::shr_abs(const Integer &a, const uint64_t b, Integer &out) {
  shr_abs_(a.abs_val_, b, out.abs_val_);
  out.neg_ = !out.zero() && a.neg_;
}
}