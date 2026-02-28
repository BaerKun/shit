#include "internal.hpp"
#include <cstring>

namespace lll {
using namespace internal;

static uint64_t ctz64(const uint64_t n) {
#ifdef _MSC_VER
  unsigned long res;
  _BitScanForward64(&res, n);
  return res;
#elif defined(__GNUC__) || defined(__clang__)
  return __builtin_ctzll(n);
#endif
}

uint64_t Integer::pow_of_2() const {
  uint64_t i, res = 0;
  for (i = 0; i < abs_val_.size() && !abs_val_[i]; i++) res += 64;
  return res + ctz64(abs_val_[i]);
}

uint64_t Integer::abs_log2() const {
  if (abs_val_.empty()) return 0;
  return abs_val_.size() * 64 - clz64(abs_val_.back()) - 1;
}

static void shl_abs_impl(const VecU64 &a, const uint64_t b, VecU64 &res) {
  const size_t size_a = a.size();
  const size_t shift_limb = b / 64;
  const size_t shift_bit = b % 64;

  res.resize(size_a + shift_limb + 1);
  memset(res.data(), 0, shift_limb * sizeof(uint64_t));

  if (shift_bit == 0) {
    memcpy(res.data() + shift_limb, a.data(), size_a * sizeof(uint64_t));
    res.pop_back();
  } else {
    uint64_t carry = 0;
    for (size_t i = 0; i < size_a; i++) {
      const uint64_t curr = a[i];
      res[i + shift_limb] = curr << shift_bit | carry;
      carry = curr >> (64 - shift_bit);
    }
    if (carry) {
      res.back() = carry;
    } else {
      res.pop_back();
    }
  }
}

void shl_abs_(const VecU64 &a, const uint64_t b, VecU64 &out) {
  if (a.empty()) {
    out.clear();
    return;
  }
  if (b == 0) {
    out = a;
    return;
  }

  if (a.data() == out.data()) {
    VecU64 res;
    shl_abs_impl(a, b, res);
    out = std::move(res);
  } else {
    shl_abs_impl(a, b, out);
  }
}

static void shr_abs_impl(const VecU64 &a, const uint64_t b, VecU64 &res) {
  const size_t size_a = a.size();
  const size_t shift_limb = b / 64;
  const size_t shift_bit = b % 64;
  const size_t size_o = size_a - shift_limb;

  res.resize(size_o);
  if (shift_bit == 0) {
    memcpy(res.data(), a.data() + shift_limb,
           (size_a - shift_limb) * sizeof(uint64_t));
  } else {
    uint64_t carry = 0;
    for (size_t i = size_o; i--;) {
      const uint64_t curr = a[i + shift_limb];
      res[i] = curr >> shift_bit | carry;
      carry = curr << (64 - shift_bit);
    }
    norm_top(res);
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

  if (a.data() == out.data()) {
    VecU64 res;
    shr_abs_impl(a, b, res);
    out = std::move(res);
  } else {
    shr_abs_impl(a, b, out);
  }
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