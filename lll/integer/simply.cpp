#include "internal.hpp"

namespace lll {
using namespace internal;

void neg(const Integer &a, Integer &out) {
  out.neg = !(a.zero() || a.neg);
  out.abs_val = a.abs_val;
}

static int ucmpr(const VecU64 &a, const VecU64 &b) {
  const size_t size_a = a.size();
  const size_t size_b = b.size();

  if (size_a > size_b) return 1;
  if (size_b > size_a) return -1;

  for (size_t i = size_a; i--;) {
    if (a[i] > b[i]) return 1;
    if (a[i] < b[i]) return -1;
  }
  return 0;
}

int cmpr(const Integer &a, const Integer &b) {
  if (a.neg) {
    if (b.neg) return ucmpr(b.abs_val, a.abs_val);
    return -1;
  }
  if (b.neg) return 1;
  return ucmpr(a.abs_val, b.abs_val);
}

static void uadd(const VecU64 &a, const VecU64 &b, VecU64 &out) {
  const VecU64 &max = a.size() > b.size() ? a : b;
  const VecU64 &min = a.size() > b.size() ? b : a;
  const size_t size_max = max.size();
  const size_t size_min = min.size();
  uint64_t carry = 0;

  out.resize(size_max);
  for (size_t i = 0; i < size_min; i++) {
    out[i] = add64(max[i], min[i], carry, carry);
  }

  for (size_t i = size_min; i < size_max; i++) {
    if (carry == 0) {
      memcpy(out.data() + i, max.data() + i, (size_max - i) * 8);
      return;
    }
    out[i] = add64(max[i], 0, carry, carry);
  }
  if (carry) out.push_back(1);
}

static void usub(const VecU64 &max, const VecU64 &min, VecU64 &out) {
  const size_t size_max = max.size();
  const size_t size_min = min.size();
  uint64_t borrow = 0;

  out.resize(size_max);
  for (size_t i = 0; i < size_min; i++) {
    out[i] = sub64(max[i], min[i], borrow, borrow);
  }

  if (size_max == size_min) {
    norm(out);
  } else {
    for (size_t i = size_min; i < size_max; i++) {
      if (borrow == 0) {
        memcpy(out.data() + i, max.data() + i, (size_max - i) * 8);
        return;
      }
      out[i] = sub64(max[i], 0, borrow, borrow);
    }
    norm_top(out);
  }
}

void add(const Integer &a, const Integer &b, Integer &out) {
  const VecU64 &abs_a = a.abs_val;
  const VecU64 &abs_b = b.abs_val;

  if (a.neg == b.neg) {
    uadd(abs_a, abs_b, out.abs_val);
    out.neg = a.neg;
  } else {
    switch (ucmpr(abs_a, abs_b)) {
    case 1:
      out.neg = a.neg;
      usub(abs_a, abs_b, out.abs_val);
      break;
    case -1:
      out.neg = b.neg;
      usub(abs_b, abs_a, out.abs_val);
      break;
    default: // 0
      out = 0;
    }
  }
}

void sub(const Integer &a, const Integer &b, Integer &out) {
  const VecU64 &abs_a = a.abs_val;
  const VecU64 &abs_b = b.abs_val;

  if (a.neg != b.neg) {
    uadd(abs_a, abs_b, out.abs_val);
    out.neg = a.neg;
  } else {
    switch (ucmpr(abs_a, abs_b)) {
    case 1:
      out.neg = a.neg;
      usub(abs_a, abs_b, out.abs_val);
      break;
    case -1:
      out.neg = !b.neg;
      usub(abs_b, abs_a, out.abs_val);
      break;
    default:
      out = 0;
    }
  }
}
} // namespace lll