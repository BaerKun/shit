#ifndef INTEGER_H
#define INTEGER_H

#include <iostream>
#include <cstdint>
#include <string>
#include <vector>

namespace lll {
class Integer;

// 1: a > b; 0: a == b; -1: a < b
int cmpr(const Integer &a, const Integer &b);
// -a
void neg(const Integer &a, Integer &out);
// a + b
void add(const Integer &a, const Integer &b, Integer &out);
// a - b
void sub(const Integer &a, const Integer &b, Integer &out);
// a * b
void mul(const Integer &a, const Integer &b, Integer &out);
// quot = a / b, rem = a % b
void div(const Integer &a, const Integer &b, Integer &quot, Integer &rem);
// pow(a, b)
void pow(const Integer &a, uint64_t b, Integer &out);

void add_64bits(const Integer &a, int64_t b, Integer &out);
void sub_64bits(const Integer &a, int64_t b, Integer &out);
void mul_64bits(const Integer &a, int64_t b, Integer &out);
void div_64bits(const Integer &a, int64_t b, Integer &quot, int64_t &rem);

class Integer {
public:
  std::vector<uint64_t> abs_val;
  bool neg = false;

  Integer(const Integer &other) = default;
  Integer(Integer &&other) = default;
  Integer &operator=(const Integer &other) = default;
  Integer &operator=(Integer &&other) = default;

  explicit Integer(const std::string &value) {
    *this = from_string(value);
  }

  Integer(const int64_t value = 0) // NOLINT(*-explicit-constructor)
    : abs_val(value != 0, static_cast<uint64_t>(std::abs(value))),
      neg(value < 0) {
  }

  Integer &operator=(const int64_t value) {
    abs_val.assign(value != 0, static_cast<uint64_t>(std::abs(value)));
    neg = value < 0;
    return *this;
  }

  bool zero() const { return abs_val.empty(); }

  std::string to_string() const;
  static Integer from_string(const std::string &str);

  Integer divide(const Integer &other, Integer &rem) const {
    Integer quot;
    div(*this, other, quot, rem);
    return quot;
  }

  Integer operator+(const Integer &other) const {
    Integer out;
    add(*this, other, out);
    return out;
  }

  Integer operator-(const Integer &other) const {
    Integer out;
    sub(*this, other, out);
    return out;
  }

  Integer operator*(const Integer &other) const {
    Integer out;
    mul(*this, other, out);
    return out;
  }

  Integer operator/(const Integer &other) const {
    Integer quot, rem;
    div(*this, other, quot, rem);
    return quot;
  }

  Integer operator%(const Integer &other) const {
    Integer quot, rem;
    div(*this, other, quot, rem);
    return rem;
  }

  friend std::istream &operator>>(std::istream &is, Integer &a);

  friend std::ostream &operator<<(std::ostream &os, const Integer &a) {
    return os << a.to_string();
  }
};

} // namespace lll

#endif // INTEGER_H