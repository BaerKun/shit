#include "internal.hpp"

namespace lll {
using namespace internal;

int cmpr(const Integer &a, const Integer &b) {
  if (a.neg_) {
    if (b.neg_) return ucmpr(b.abs_val_, a.abs_val_);
    return -1;
  }
  if (b.neg_) return 1;
  return ucmpr(a.abs_val_, b.abs_val_);
}

void neg(const Integer &a, Integer &out) {
  out.neg_ = !(a.zero() || a.neg_);
  out.abs_val_ = a.abs_val_;
}

void add(const Integer &a, const Integer &b, Integer &out) {
  const VecU64 &abs_a = a.abs_val_;
  const VecU64 &abs_b = b.abs_val_;
  const int ucmpr_res = ucmpr(abs_a, abs_b);

  if (ucmpr_res == 0 && a.neg_ != b.neg_) {
    out = 0;
  } else if (ucmpr_res > 0) {
    if (a.neg_ == b.neg_) {
      uadd(abs_a, abs_b, out.abs_val_);
    } else {
      usub(abs_a, abs_b, out.abs_val_);
    }
    out.neg_ = a.neg_;
  } else {
    if (a.neg_ == b.neg_) {
      uadd(abs_b, abs_a, out.abs_val_);
    } else {
      usub(abs_b, abs_a, out.abs_val_);
    }
    out.neg_ = b.neg_;
  }
}

void sub(const Integer &a, const Integer &b, Integer &out) {
  const VecU64 &abs_a = a.abs_val_;
  const VecU64 &abs_b = b.abs_val_;
  const int ucmpr_res = ucmpr(abs_a, abs_b);

  if (ucmpr_res == 0 && a.neg_ == b.neg_) {
    out = 0;
  } else if (ucmpr_res > 0) {
    if (a.neg_ == b.neg_) {
      usub(abs_a, abs_b, out.abs_val_);
    } else {
      uadd(abs_a, abs_b, out.abs_val_);
    }
    out.neg_ = a.neg_;
  } else {
    if (a.neg_ == b.neg_) {
      usub(abs_b, abs_a, out.abs_val_);
    } else {
      uadd(abs_b, abs_a, out.abs_val_);
    }
    out.neg_ = !b.neg_;
  }
}
} // namespace lll