#include "internal.hpp"

#include <cmath>

namespace lll {
using namespace internal;

// log10(2^64)
#define LOG10_2_64 std::log10(18446744073709551616.)

// MAX_UINT64 = 18446744073709551615;
constexpr uint64_t MAX_DIGITS = 19;
constexpr uint64_t BASE = 10000000000000000000llu; // 10^19

static size_t to_string_base(uint64_t n, char *ptr, const bool leading_zero) {
  size_t i;
  for (i = 0; n; i++) {
    const uint64_t digit = n % 10;
    n /= 10;
    ptr[i] = static_cast<char>(digit + '0');
  }
  if (leading_zero) {
    memset(ptr, '0', i + 1);
    return MAX_DIGITS;
  }
  return i; // len
}

std::string Integer::to_string() const {
  if (zero()) return "0";

  const size_t size = abs_val.size();
  std::string str(
      static_cast<size_t>(static_cast<double>(size) * LOG10_2_64) + 3, '\0');
  char *ptr = &str[0];

  if (size == 1) {
    const uint64_t q = abs_val[0] / BASE;
    const uint64_t r = abs_val[0] % BASE;
    const bool top = q != 0;
    const size_t len = to_string_base(r, ptr, top);
    if (top) ptr[MAX_DIGITS] = '1';
    if (neg) ptr[len + top] = '-';
    std::reverse(str.begin(), str.begin() + (len + top + neg));
  } else {
    Integer quot = *this;
    uint64_t rem = 0;
    size_t i, len = 0;
    for (i = 0; !quot.zero(); i++) {
      div_64bits(quot, BASE, quot, rem);
      len = to_string_base(rem, ptr + i * MAX_DIGITS, !quot.zero());
    }
    if (neg) ptr[i * MAX_DIGITS + len] = '-';
    std::reverse(str.begin(), str.begin() + (i * MAX_DIGITS + len + neg));
  }
  return str;
}
}