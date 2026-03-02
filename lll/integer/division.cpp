#include "internal.hpp"
#include <stdexcept>

namespace lll {
using namespace internal;

// quotient should be always 64-bits.
static inline uint64_t div128(const uint64_t high, const uint64_t low,
                              const uint64_t divisor, uint64_t &rem) {
#if defined(_MSC_VER)
  return _udiv128(high, low, divisor, &rem);
#elif defined(__GNUC__) || defined(__clang__)
  const __uint128_t dividend = (__uint128_t)high << 64 | (__uint128_t)low;
  rem = dividend % divisor;
  return dividend / divisor;
#else

#endif
}

void internal::udiv_64bits_(const VecU64 &ddd, const uint64_t dsr, VecU64 &quot,
                            uint64_t &rem) {
  const size_t size = ddd.size();
  rem = 0;
  quot.resize(size);
  for (size_t i = size; i--;) {
    quot[i] = div128(rem, ddd[i], dsr, rem);
  }
  norm_top(quot);
}

static uint64_t umod_64bits_(const VecU64 &ddd, const uint64_t dsr) {
  const size_t size = ddd.size();
  uint64_t rem = 0;
  for (size_t i = size; i--;) {
    div128(rem, ddd[i], dsr, rem);
  }
  return rem;
}

static uint64_t
shift_left(const VecU64 &src, VecU64 &dst, const uint64_t shift) {
  uint64_t carry = 0;
  for (size_t i = 0; i < src.size(); i++) {
    const uint64_t curr = src[i];
    dst[i] = curr << shift | carry;
    carry = curr >> (64 - shift);
  }
  return carry;
}

static void shift_operands(const VecU64 &dividend, const VecU64 &divisor,
                           VecU64 &ddd, VecU64 &dsr, const uint64_t shift) {
  if (shift == 0) {
    ddd = dividend;
    dsr = divisor;
    ddd.push_back(0);
    return;
  }

  ddd.resize(dividend.size() + 1);
  ddd.back() = shift_left(dividend, ddd, shift);

  dsr.resize(divisor.size());
  shift_left(divisor, dsr, shift);
}

static void div_lll_main(VecU64 &ddd, const VecU64 &dsr, VecU64 *quot) {
  const size_t m = ddd.size();
  const size_t n = dsr.size();

  const uint64_t dsr_top1 = dsr[n - 1];
  const uint64_t dsr_top2 = dsr[n - 2];
  uint64_t q_hat, r_hat, high, low;
  for (size_t i = m - n; i--;) {
    q_hat = div128(ddd[i + n], ddd[i + n - 1], dsr_top1, r_hat);

  again:
    mul64(q_hat, dsr_top2, high, low);
    // q_hat * divisor_top2 > r_hat << 64 | dividend[i + n - 2]
    if (high > r_hat || (high == r_hat && low > ddd[i + n - 2])) {
      q_hat -= 1;
      r_hat += dsr_top1;
      // r_hat didn't overflow; go to 'again' at most once.
      if (r_hat >= dsr_top1) goto again;
    }

    uint64_t borrow = 0;
    for (size_t j = 0; j < n; j++) {
      mul64(q_hat, dsr[j], high, low);
      ddd[i + j] = sub64(ddd[i + j], low, borrow, borrow);
      borrow += high;
    }
    ddd[i + n] -= borrow;

    if (ddd[i + n]) { // ddd[i + n] < 0
      q_hat -= 1;
      uint64_t carry = 0;
      for (size_t j = 0; j < n; j++) {
        ddd[i + j] = add64(ddd[i + j], dsr[j], carry, carry);
      }
      ddd[i + n] += carry;
    }
    if (quot) quot->at(i) = q_hat;
  }
}

static void pop_rem_lll(VecU64 &ddd, VecU64 &rem, const uint64_t shift) {
  norm(ddd);
  if (shift == 0) {
    rem = ddd;
    return;
  }

  const size_t size = ddd.size();
  rem.resize(size);

  uint64_t carry = 0;
  for (size_t i = size; i--;) {
    const uint64_t curr = ddd[i];
    rem[i] = curr >> shift | carry;
    carry = curr << (64 - shift);
  }
  norm_top(rem);
}

static void div_lll(const VecU64 &dividend, const VecU64 &divisor, VecU64 &quot,
                    VecU64 *rem) {
  const int cmp_res = ucmp(dividend, divisor);
  if (cmp_res == 0) {
    assign64(quot, 1);
    if (rem) rem->clear();
  }
  if (cmp_res < 0) {
    assign64(quot, 0);
    if (rem) *rem = dividend;
    return;
  }

  VecU64 ddd, dsr;
  const uint64_t shift = clz64(divisor.back());
  shift_operands(dividend, divisor, ddd, dsr, shift);

  quot.resize(ddd.size() - dsr.size());
  div_lll_main(ddd, dsr, &quot);
  norm_top(quot);

  if (rem) pop_rem_lll(ddd, *rem, shift);
}

static void mod_lll(const VecU64 &dividend, const VecU64 &divisor,
                    VecU64 &rem) {
  const int cmp_res = ucmp(dividend, divisor);
  if (cmp_res == 0) {
    rem.clear();
    return;
  }
  if (cmp_res < 0) {
    rem = dividend;
    return;
  }

  VecU64 ddd, dsr;
  const uint64_t shift = clz64(divisor.back());
  shift_operands(dividend, divisor, ddd, dsr, shift);
  div_lll_main(ddd, dsr, nullptr);
  pop_rem_lll(ddd, rem, shift);
}

void Integer::div_64bits(const Integer &a, const int64_t b, Integer &quot,
                         int64_t *rem) {
  if (b == 0) throw std::domain_error("Division by zero");

  uint64_t rem_u;
  udiv_64bits_(a.abs_val_, abs64(b), quot.abs_val_, rem_u);

  quot.neg_ = !quot.zero() && a.neg_ ^ (b < 0);
  if (rem) *rem = static_cast<int64_t>(a.neg_ ? -rem_u : rem_u);
}

void Integer::div(const Integer &a, const Integer &b, Integer &quot,
                  Integer *rem) {
  if (b.zero()) throw std::domain_error("Division by zero");

  const VecU64 &dividend = a.abs_val_;
  const VecU64 &divisor = b.abs_val_;

  if (divisor.size() == 1) {
    uint64_t rem_64bits;
    udiv_64bits_(dividend, divisor[0], quot.abs_val_, rem_64bits);
    if (rem) assign64(rem->abs_val_, rem_64bits);
  } else {
    div_lll(dividend, divisor, quot.abs_val_, rem ? &rem->abs_val_ : nullptr);
  }

  quot.neg_ = !quot.zero() && a.neg_ ^ b.neg_;
  if (rem) rem->neg_ = !rem->zero() && a.neg_;
}

void Integer::mod_64bits(const Integer &a, const int64_t b, int64_t &out) {
  if (b == 0) throw std::domain_error("Division by zero");

  const uint64_t rem_u = umod_64bits_(a.abs_val_, abs64(b));
  out = static_cast<int64_t>(a.neg_ ? -rem_u : rem_u);
}

void Integer::mod(const Integer &a, const Integer &b, Integer &out) {
  if (b.zero()) throw std::domain_error("Division by zero");

  const VecU64 &dividend = a.abs_val_;
  const VecU64 &divisor = b.abs_val_;

  if (divisor.size() == 1) {
    const uint64_t rem_64bits = umod_64bits_(dividend, divisor[0]);
    assign64(out.abs_val_, rem_64bits);
  } else {
    mod_lll(dividend, divisor, out.abs_val_);
  }

  out.neg_ = !out.zero() && a.neg_;
}
} // namespace lll