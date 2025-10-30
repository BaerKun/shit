#include "internal.hpp"
#include <stdexcept>

namespace lll {
using namespace internal;

// n != 0
static inline unsigned nlz64(uint64_t n) {
  unsigned m = 1;
  // clang-format off
  if (n >> 32 == 0) { m += 32; n <<= 32; }
  if (n >> 48 == 0) { m += 16; n <<= 16; }
  if (n >> 56 == 0) { m += 8;  n <<= 8;  }
  if (n >> 60 == 0) { m += 4;  n <<= 4;  }
  if (n >> 62 == 0) { m += 2;  n <<= 2;  }
  // clang-format on
  return m - (n >> 63);
}

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

void internal::div_64bits_internal(const VecU64 &dividend,
                                   const uint64_t divisor,
                                   VecU64 &quot, uint64_t &rem) {
  VecU64 quot_tmp(dividend.size());
  uint64_t high = 0;
  for (size_t i = dividend.size(); i--;) {
    quot_tmp[i] = div128(high, dividend[i], divisor, high);
  }
  norm(quot_tmp);
  quot = std::move(quot_tmp);
  rem = high;
}

void div_64bits(const Integer &a, const int64_t b, Integer &quot,
                int64_t &rem) {
  if (b == 0) throw std::runtime_error("Division by zero");

  uint64_t rem_u;
  div_64bits_internal(a.abs_val, std::abs(b), quot.abs_val, rem_u);

  quot.neg = !quot.zero() && a.neg ^ (b < 0);
  if (a.neg) rem_u = -rem_u;
  rem = static_cast<int64_t>(rem_u);
}

static void divided_by_lll(const VecU64 &dividend, const VecU64 &divisor,
                           VecU64 &quot, VecU64 &rem) {
  const size_t m = dividend.size();
  const size_t n = divisor.size();
  const unsigned shift = nlz64(divisor[n - 1]);
  VecU64 ddd(m + 1), dsr(n);

  ddd[m] = dividend[m - 1] >> (64 - shift);
  for (size_t i = m - 1; i; i--) {
    ddd[i] = (dividend[i] << shift) | (dividend[i - 1] >> (64 - shift));
  }
  ddd[0] = dividend[0] << shift;

  for (size_t i = n - 1; i; i--) {
    dsr[i] = (divisor[i] << shift) | (divisor[i - 1] >> (64 - shift));
  }
  dsr[0] = divisor[0] << shift;

  quot.resize(m - n + 1);

  const uint64_t dsr_top1 = dsr[n - 1];
  const uint64_t dsr_top2 = dsr[n - 2];
  uint64_t q_hat, r_hat, high, low;
  size_t i = m - n;
  do {
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

    if (ddd[i + n] < 0) {
      q_hat -= 1;
      uint64_t carry = 0;
      for (size_t j = 0; j < n; j++) {
        ddd[i + j] = add64(ddd[i + j], dsr[j], carry, carry);
      }
      ddd[i + n] += carry;
    }
    quot[i] = q_hat;
  } while (i--);

  rem.resize(n);
  for (i = 0; i < n; i++) {
    rem[i] = (ddd[i] >> shift) | (ddd[i + 1] << (64 - shift));
  }

  norm_top(quot);
  norm(rem);
}

static void div_impl(const VecU64 &dividend, const VecU64 &divisor,
                     VecU64 &quot, VecU64 &rem) {
  const size_t m = dividend.size();
  const size_t n = divisor.size();

  if (m < n) {
    assign64(quot, 0);
    rem = dividend;
    return;
  }

  if (n == 1) {
    uint64_t rem_64bits;
    div_64bits_internal(dividend, divisor[0], quot, rem_64bits);
    assign64(rem, rem_64bits);
  } else {
    divided_by_lll(dividend, divisor, quot, rem);
  }
}

void div(const Integer &a, const Integer &b, Integer &quot, Integer &rem) {
  if (b.zero()) throw std::runtime_error("Division by zero");

  div_impl(a.abs_val, b.abs_val, quot.abs_val, rem.abs_val);

  quot.neg = !quot.zero() && a.neg ^ b.neg;
  rem.neg = !rem.zero() && a.neg;
}
} // namespace lll