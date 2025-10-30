#include "internal.hpp"

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
    memset(ptr + i, '0', MAX_DIGITS - i);
    return MAX_DIGITS;
  }
  return i; // len
}

std::string Integer::to_string() const {
  if (zero()) return "0";

  const size_t size = abs_val.size();
  if (size == 1) {
    std::string abs = std::to_string(abs_val[0]);
    if (neg) return "-" + abs;
    return abs;
  }

  std::string str(
      static_cast<size_t>(static_cast<double>(size) * LOG10_2_64) + 3, '\0');
  char *ptr = &str[0];

  VecU64 quot = abs_val;
  uint64_t rem = 0;
  size_t i = -1, len = 0;
  do {
    div_64bits_internal(quot, BASE, quot, rem);
    len = to_string_base(rem, ptr + ++i * MAX_DIGITS, !quot.empty());
  } while (!quot.empty());
  if (neg) ptr[i * MAX_DIGITS + len] = '-';

  str.resize(i * MAX_DIGITS + len + neg);
  std::reverse(str.begin(),
               str.begin() + static_cast<int64_t>(str.size()));
  return str;
}
}