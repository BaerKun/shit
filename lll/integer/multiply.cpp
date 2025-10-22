#include "internal.hpp"

namespace lll {
using namespace internal;

void mul(const Integer &a, const Integer &b, Integer &out) {
  const VecU64 &abs_a = a.abs_val_;
  const VecU64 &abs_b = b.abs_val_;
  const size_t size_a = abs_a.size();
  const size_t size_b = abs_b.size();
  VecU64 res(abs_a.size() + abs_b.size(), 0);

  for (size_t i = 0; i < size_a; i++) {
    uint64_t low, high = 0;
    for (size_t j = 0; j < size_b; j++) {
      uint64_t &sum = res[i + j];
      sum += high;
      const uint64_t carry1 = sum < high;
      mul64(abs_a[i], abs_b[j], low, high);
      sum += low;
      const uint64_t carry2 = sum < low;
      high += carry1 + carry2;
    }
    res[i + size_b] = high;
  }

  out.abs_val_ = std::move(res);
  out.neg_ = a.neg_ ^ b.neg_;
}
} // namespace lll