#include "internal.hpp"

namespace lll {
namespace internal {
int ucmpr(const VecU64 &a, const VecU64 &b) {
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

void uadd(const VecU64 &max, const VecU64 &min, VecU64 &out) {
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

void usub(const VecU64 &max, const VecU64 &min, VecU64 &out) {
  const size_t size_max = max.size();
  const size_t size_min = min.size();
  uint64_t borrow = 0;

  out.resize(size_max);
  for (size_t i = 0; i < size_min; i++) {
    out[i] = sub64(max[i], min[i], borrow, borrow);
  }

  if (size_max == size_min) {
    for (size_t i = size_max; --i != 0;) {
      if (out[i] == 0) out.pop_back();
    }
  } else {
    for (size_t i = size_min; i < size_max; i++) {
      if (borrow == 0) {
        memcpy(out.data() + i, max.data() + i, (size_max - i) * 8);
        return;
      }
      out[i] = sub64(max[i], 0, borrow, borrow);
    }
    if (out[size_max - 1] == 0) out.pop_back();
  }
}
}
}