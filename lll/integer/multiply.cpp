#include "internal.hpp"

namespace lll {
using namespace internal;

void mul(const Integer &a, const Integer &b, Integer &out) {
  const VecU64 &abs_a = a.abs_val_;
  const VecU64 &abs_b = b.abs_val_;
  VecU64 result(abs_a.size() + abs_b.size());

  out.neg_ = a.neg_ ^ b.neg_;
}
}