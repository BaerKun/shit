#include "internal.hpp"
#include <cmath>

namespace lll {
using namespace internal;

// MAX_UINT64 = 18446744073709551615;
constexpr uint64_t MAX_DIGITS = 19;
constexpr uint64_t BASE = 10000000000000000000llu; // 10^19

static uint64_t *base_cache() {
  static uint64_t cache[19];
  cache[0] = 1;
  for (size_t i = 1; i < MAX_DIGITS; i++) cache[i] = cache[i - 1] * 10;
  return cache;
}

static uint64_t from_string_base(const char *src, const size_t len) {
  uint64_t n = 0;
  for (size_t i = 0; i < len; i++) n = n * 10 + src[i] - '0';
  return n;
}

static void from_string_u(VecU64 &abs, const char *ptr, size_t size) {
  static const uint64_t *base_10_ = base_cache();
  while (size > MAX_DIGITS) {
    umul_64bits_(abs, BASE, abs);
    uadd_64bits_(abs, from_string_base(ptr, MAX_DIGITS), abs);
    ptr += MAX_DIGITS;
    size -= MAX_DIGITS;
  }
  umul_64bits_(abs, base_10_[size], abs);
  uadd_64bits_(abs, from_string_base(ptr, size), abs);
}

Integer::Integer(const std::string &value) {
  if (value.empty()) return;
  const char *ptr = value.data();
  size_t size = value.size();
  switch (*ptr) {
  case '-':
    neg_ = true;
  case '+':
    ptr++;
    size--;
  default:
    from_string_u(abs_val_, ptr, size);
  }
}

static size_t to_string_base(uint64_t n, char *ptr, const bool leading_zero) {
  size_t len = 0;
  do ptr[len++] = static_cast<char>('0' + n % 10); while (n /= 10);
  if (!leading_zero) return len;
  memset(ptr + len, '0', MAX_DIGITS - len);
  return MAX_DIGITS;
}

static size_t to_string_u_rev(const VecU64 &abs, char *dst) {
  const size_t size = abs.size();
  if (size == 1) return to_string_base(abs[0], dst, false);

  VecU64 quot = abs;
  uint64_t rem = 0, len = 0;
  for (char *p = dst;;) {
    udiv_64bits_(quot, BASE, quot, rem);
    const bool quot_zero = quot.empty();
    len += to_string_base(rem, p, !quot_zero);
    p += MAX_DIGITS;
    if (quot_zero) break;
  }
  return len;
}

std::string Integer::to_string() const {
  if (zero()) return "0";

  // log10(2^64)
  static const double LOG10_2_64 = std::log10(18446744073709551616.);
  const size_t size = abs_val_.size();
  std::string str((size_t)((double)size * LOG10_2_64) + 3, '\0');
  char *ptr = &str[0];
  if (neg_) {
    ptr[0] = '-';
    ptr++;
  }

  const size_t len = to_string_u_rev(abs_val_, ptr);
  str.resize(len + neg_);
  std::reverse(ptr, ptr + len);
  return str;
}
}