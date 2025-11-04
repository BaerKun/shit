#include "internal.hpp"

namespace lll {
using namespace internal;

// log10(2^64)
#define LOG10_2_64 std::log10(18446744073709551616.)

// MAX_UINT64 = 18446744073709551615;
constexpr uint64_t MAX_DIGITS = 19;
constexpr uint64_t BASE = 10000000000000000000llu; // 10^19

Integer::Integer(const std::string &value) {

}

static size_t to_string_base(uint64_t n, char *ptr, const bool leading_zero) {
  size_t i = 0;
  do {
    ptr[i++] = static_cast<char>('0' + n % 10);
    n /= 10;
  } while (n);
  if (leading_zero) {
    memset(ptr + i, '0', MAX_DIGITS - i);
    return MAX_DIGITS;
  }
  return i; // len
}

std::string Integer::to_string() const {
  if (zero()) return "0";

  const size_t size = abs_val_.size();
  if (size == 1) {
    std::string abs = std::to_string(abs_val_[0]);
    if (neg_) return "-" + abs;
    return abs;
  }

  std::string str(
      static_cast<size_t>(static_cast<double>(size) * LOG10_2_64) + 3, '\0');
  char *ptr = &str[0];

  VecU64 quot = abs_val_;
  uint64_t rem = 0, len = 0;
  for (char *p = ptr;;) {
    udiv_64bits_(quot, BASE, quot, rem);
    const bool quot_zero = quot.empty();
    len += to_string_base(rem, p, !quot_zero);
    p += MAX_DIGITS;
    if (quot_zero) break;
  }
  if (neg_) ptr[len] = '-';

  str.resize(len + neg_);
  std::reverse(str.begin(),
               str.begin() + static_cast<int64_t>(str.size()));
  return str;
}
}