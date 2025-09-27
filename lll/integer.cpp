#include "integer.hpp"

namespace lll {
using DataType = std::vector<uint64_t>;

static inline void add64(const uint64_t a, const uint64_t b, uint64_t &carry,
                         uint64_t &out) {
  const uint64_t sum = a + b + carry;
  carry = sum < a || sum < b;
  out = sum;
}

static int ucmpr(const std::vector<uint64_t> &a,
const std::vector<uint64_t> &b) {
  const size_t size_a = a.size();
  const size_t size_b = b.size();

  if (size_a > size_b)
    return 1;
  if(size_b > size_a)
    return -1;

  for(size_t i = size_a; i--;) {
    if(a[i] > b[i]) return 1;
    if(a[i] < b[i]) return -1;
  }
  return 0;
}

int cmpr(const Integer &a, const Integer &b) {
  if(a.neg_) {
    if(b.neg_) return ucmpr(b.data_, a.data_);
    return -1;
  }
  if(b.neg_) return 1;
  return ucmpr(a.data_, b.data_);
}

void neg(const Integer &a, Integer &out) {
  out.neg_ = !a.neg_;
  out.data_ = a.data_;
}

static void uadd(const std::vector<uint64_t> &max,
                 const std::vector<uint64_t> &min, std::vector<uint64_t> &out) {
  const size_t size_max = max.size();
  const size_t size_min = min.size();
  uint64_t carry = 0;

  out.resize(size_max);
  for (size_t i = 0; i < size_min; i++) {
    add64(max[i], min[i], carry, out[i]);
  }
  for (size_t i = size_min; i < size_max; i++) {
    if (carry == 0) {
      memcpy(out.data() + i, max.data() + i, (size_max - i) * 8);
      return;
    }
    add64(max[i], 0, carry, out[i]);
  }
  if (carry) out.push_back(1);
}

static void usub(const std::vector<uint64_t> &max,
                 const std::vector<uint64_t> &min, std::vector<uint64_t> &out) {
}

void add(const Integer &a, const Integer &b, Integer &out) {
  const int ucmpr_res = ucmpr(a.data_, b.data_);
  const Integer &umax = ucmpr_res > 0 ? a : b;
  const Integer &umin = ucmpr_res > 0 ? b : a;

  if(umax.neg_ == umin.neg_) {
    uadd(umax.data_, umin.data_, out.data_);
  } else {
    if(ucmpr_res == 0) {
      out = Integer(0);
      return;
    }
    usub(umax.data_, umin.data_, out.data_);
  }
  out.neg_ = umax.neg_;
}

} // namespace lll