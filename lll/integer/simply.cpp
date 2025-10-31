#include "internal.hpp"

namespace lll {
using namespace internal;

void Integer::opp(const Integer &a, Integer &out) {
  out.neg_ = !(a.zero() || a.neg_);
  out.abs_val_ = a.abs_val_;
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

int Integer::cmpr(const Integer &a, const Integer &b) {
  if (a.neg_) {
    if (b.neg_) return ucmpr(b.abs_val_, a.abs_val_);
    return -1;
  }
  if (b.neg_) return 1;
  return ucmpr(a.abs_val_, b.abs_val_);
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

  for (size_t i = size_min; i < size_max; i++) {
    if (borrow == 0) {
      memcpy(out.data() + i, max.data() + i, (size_max - i) * 8);
      return;
    }
    out[i] = sub64(max[i], 0, borrow, borrow);
  }

  if (size_max <= size_min + 1) norm(out);
  else norm_top(out);
}

void Integer::add(const Integer &a, const Integer &b, Integer &out) {
  const VecU64 &abs_a = a.abs_val_;
  const VecU64 &abs_b = b.abs_val_;

  if (a.neg_ == b.neg_) {
    uadd(abs_a, abs_b, out.abs_val_);
    out.neg_ = a.neg_;
  } else {
    switch (ucmpr(abs_a, abs_b)) {
    case 1:
      out.neg_ = a.neg_;
      usub(abs_a, abs_b, out.abs_val_);
      break;
    case -1:
      out.neg_ = b.neg_;
      usub(abs_b, abs_a, out.abs_val_);
      break;
    default: // 0
      out = 0;
    }
  }
}

void Integer::sub(const Integer &a, const Integer &b, Integer &out) {
  const VecU64 &abs_a = a.abs_val_;
  const VecU64 &abs_b = b.abs_val_;

  if (a.neg_ != b.neg_) {
    uadd(abs_a, abs_b, out.abs_val_);
    out.neg_ = a.neg_;
  } else {
    switch (ucmpr(abs_a, abs_b)) {
    case 1:
      out.neg_ = a.neg_;
      usub(abs_a, abs_b, out.abs_val_);
      break;
    case -1:
      out.neg_ = !b.neg_;
      usub(abs_b, abs_a, out.abs_val_);
      break;
    default:
      out = 0;
    }
  }
}
} // namespace lll