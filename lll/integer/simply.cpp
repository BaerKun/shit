#include "internal.hpp"

namespace lll {
using namespace internal;

void Integer::opp(const Integer &a, Integer &out) {
  out.neg_ = !(a.zero() || a.neg_);
  out.abs_val_ = a.abs_val_;
}

static int ucmp_64bits_(const VecU64 &a, const uint64_t b) {
  const size_t size_a = a.size();
  const size_t size_b = b != 0;

  if (size_a > size_b) return 1;
  if (size_b > size_a) return -1;
  return (a[0] >= b) - (a[0] <= b);
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

int Integer::cmp_64bits(const Integer &a, const int64_t b) {
  const VecU64 &abs_a = a.abs_val_;
  const uint64_t abs_b = std::abs(b);

  if (a.neg_) {
    if (b < 0) return -ucmp_64bits_(abs_a, abs_b);
    return -1;
  }
  if (b < 0) return 1;
  return ucmp_64bits_(abs_a, abs_b);
}


int Integer::cmp(const Integer &a, const Integer &b) {
  if (a.neg_) {
    if (b.neg_) return ucmpr(b.abs_val_, a.abs_val_);
    return -1;
  }
  if (b.neg_) return 1;
  return ucmpr(a.abs_val_, b.abs_val_);
}

void internal::uadd_64bits_(const VecU64 &a, const uint64_t b, VecU64 &out) {
  if (a.empty()) {
    assign64(out, b);
    return;
  }
  if (b == 0) {
    out = a;
    return;
  }

  const size_t size_a = a.size();
  uint64_t carry;

  out.resize(size_a + 1);
  out[0] = add64(a[0], b, 0, carry);

  for (size_t i = 1; i <= size_a; i++) {
    if (carry == 0) {
      memcpy(out.data() + i, a.data() + i, (size_a - i) * 8);
      out.pop_back();
      break;
    }
    out[i] = add64(a[i], 0, carry, carry);
  }
}

static void uadd(const VecU64 &a, const VecU64 &b, VecU64 &out) {
  const VecU64 &max = a.size() > b.size() ? a : b;
  const VecU64 &min = a.size() > b.size() ? b : a;
  const size_t size_max = max.size();
  const size_t size_min = min.size();
  uint64_t carry = 0;

  out.resize(size_max + 1);
  for (size_t i = 0; i < size_min; i++) {
    out[i] = add64(max[i], min[i], carry, carry);
  }

  for (size_t i = size_min; i <= size_max; i++) {
    if (carry) {
      memcpy(out.data() + i, a.data() + i, (size_max - i) * 8);
      out.pop_back();
      break;
    }
    out[i] = add64(a[i], 0, carry, carry);
  }
}

void internal::usub_64bits_(const VecU64 &a, const uint64_t b, VecU64 &out) {
  if (b == 0) {
    out = a;
    return;
  }

  const size_t size_a = a.size();
  uint64_t borrow;

  out.resize(size_a);
  out[0] = sub64(a[0], b, 0, borrow);

  for (size_t i = 1; i < size_a; i++) {
    if (borrow == 0) {
      memcpy(out.data() + i, a.data() + i, (size_a - i) * 8);
      return;
    }
    out[i] = sub64(a[i], 0, borrow, borrow);
  }
  norm_top(out);
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

void Integer::add_64bits(const Integer &a, const int64_t b, Integer &out) {
  const VecU64 &abs_a = a.abs_val_;
  const uint64_t abs_b = std::abs(b);
  VecU64 &abs_o = out.abs_val_;
  const bool neg_b = b < 0;

  if (a.neg_ == neg_b) {
    uadd_64bits_(abs_a, abs_b, abs_o);
    out.neg_ = a.neg_;
    return;
  }
  switch (ucmp_64bits_(abs_a, abs_b)) {
  case 1:
    usub_64bits_(abs_a, abs_b, abs_o);
    out.neg_ = a.neg_;
    break;
  case -1:
    abs_o.assign(1, abs_a.empty() ? abs_b : abs_b - abs_a[0]);
    out.neg_ = neg_b;
    break;
  default: // 0
    out = 0;
  }
}

void Integer::add(const Integer &a, const Integer &b, Integer &out) {
  const VecU64 &abs_a = a.abs_val_;
  const VecU64 &abs_b = b.abs_val_;
  VecU64 &abs_o = out.abs_val_;

  if (a.neg_ == b.neg_) {
    uadd(abs_a, abs_b, abs_o);
    out.neg_ = a.neg_;
    return;
  }
  switch (ucmpr(abs_a, abs_b)) {
  case 1:
    usub(abs_a, abs_b, abs_o);
    out.neg_ = a.neg_;
    break;
  case -1:
    usub(abs_b, abs_a, abs_o);
    out.neg_ = b.neg_;
    break;
  default: // 0
    out = 0;
  }
}

void Integer::sub_64bits(const Integer &a, const int64_t b, Integer &out) {
  const VecU64 &abs_a = a.abs_val_;
  const uint64_t abs_b = std::abs(b);
  VecU64 &abs_o = out.abs_val_;
  const bool neg_b = b < 0;

  if (a.neg_ != neg_b) {
    uadd_64bits_(abs_a, abs_b, abs_o);
    out.neg_ = a.neg_;
    return;
  }
  switch (ucmp_64bits_(abs_a, abs_b)) {
  case 1:
    usub_64bits_(abs_a, abs_b, abs_o);
    out.neg_ = a.neg_;
    break;
  case -1:
    abs_o.assign(1, abs_a.empty() ? abs_b : abs_b - abs_a[0]);
    out.neg_ = !neg_b;
    break;
  default:
    out = 0;
  }
}

void Integer::sub(const Integer &a, const Integer &b, Integer &out) {
  const VecU64 &abs_a = a.abs_val_;
  const VecU64 &abs_b = b.abs_val_;
  VecU64 &abs_o = out.abs_val_;

  if (a.neg_ != b.neg_) {
    uadd(abs_a, abs_b, abs_o);
    out.neg_ = a.neg_;
    return;
  }
  switch (ucmpr(abs_a, abs_b)) {
  case 1:
    usub(abs_a, abs_b, abs_o);
    out.neg_ = a.neg_;
    break;
  case -1:
    usub(abs_b, abs_a, abs_o);
    out.neg_ = !b.neg_;
    break;
  default:
    out = 0;
  }
}
} // namespace lll