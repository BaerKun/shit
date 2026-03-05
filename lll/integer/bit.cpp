#include "internal.hpp"

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

static uint64_t shl64(const uint64_t n, const uint64_t s, uint64_t &carry) {
  const uint64_t res = n << s | carry;
  carry = n >> (64 - s);
  return res;
}

static uint64_t shr64(const uint64_t n, const uint64_t s, uint64_t &carry) {
  const uint64_t res = n >> s | carry;
  carry = n << (64 - s);
  return res;
}

static void shl_abs_(const VecU64 &a, const uint64_t b, VecU64 &out) {
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

  if (a.data() != out.data()) out.clear();
  out.resize(size_a + shift_limb + 1);

  if (shift_bit == 0) {
    for (size_t i = size_a; i--;) out[i + shift_limb] = a[i];
    out.pop_back();
  } else {
    //    limb          bit
    // (a  <<  (Sl+1))  >>  (64 - Sb)
    uint64_t carry = 0;
    const uint64_t back = shr64(a.back(), 64 - shift_bit, carry);
    if (back) {
      out.back() = back;
    } else {
      out.pop_back();
    }

    for (size_t i = size_a; i--;) {
      out[i + shift_limb] = shr64(a[i], 64 - shift_bit, carry);
    }
  }

  for (size_t i = shift_limb; i--;) out[i] = 0;
}

static void shr_abs_(const VecU64 &a, const uint64_t b, VecU64 &out) {
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

  if (a.data() != out.data()) {
    out.clear();
    out.resize(size_o);
  }

  if (shift_bit == 0) {
    for (size_t i = 0; i < size_o; i++) out[i] = a[i + shift_limb];
    out.resize(size_o);
  } else {
    //    limb          bit
    // (a  >>  (Sl+1))  <<  (64 - Sb)
    uint64_t carry = a[shift_limb] >> shift_bit;
    for (size_t i = 1; i < size_o; i++) {
      out[i - 1] = shl64(a[i + shift_limb], 64 - shift_bit, carry);
    }

    out.resize(size_o);
    if (carry) {
      out.back() = carry;
    } else {
      out.pop_back();
    }
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