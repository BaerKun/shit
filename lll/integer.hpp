#ifndef LLL_INTEGER_H
#define LLL_INTEGER_H

#include <cstdint>
#include <string>
#include <vector>

namespace lll {
class Integer {
public:
  using VecView = std::vector<uint64_t>;

  Integer(const Integer &other) = default;
  Integer(Integer &&other) = default;
  Integer &operator=(const Integer &other) = default;
  Integer &operator=(Integer &&other) = default;

  explicit Integer(const std::string &value);
  std::string to_string() const;

  Integer(const int64_t value = 0) // NOLINT(*-explicit-constructor)
    : neg_(value < 0),
      abs_val_(value != 0, static_cast<uint64_t>(neg_ ? -value : value)) {
  }

  Integer &operator=(const int64_t value) {
    neg_ = value < 0;
    abs_val_.assign(value != 0, static_cast<uint64_t>(neg_ ? -value : value));
    return *this;
  }

  bool neg() const { return neg_; }
  bool zero() const { return abs_val_.empty(); }
  uint64_t abs_low64() const { return zero() ? 0 : abs_val_.front(); }
  uint64_t pow_of_2() const; // max{ e | x = d * 2 ^ e }
  uint64_t abs_log2() const; // floor(log2(|x|)), x != 0
  Integer abs() const { return neg_ ? -*this : *this; }
  const VecView &view_v() const { return abs_val_; }

  bool operator<(const Integer &other) const { return cmp(*this, other) < 0; }

  bool operator<(const int64_t other) const {
    return cmp_64bits(*this, other) < 0;
  }

  bool operator>(const Integer &other) const { return cmp(*this, other) > 0; }

  bool operator>(const int64_t &other) const {
    return cmp_64bits(*this, other) > 0;
  }

  bool operator==(const Integer &other) const { return cmp(*this, other) == 0; }

  bool operator==(const int64_t &other) const {
    return cmp_64bits(*this, other) == 0;
  }

  bool operator<=(const Integer &other) const { return cmp(*this, other) <= 0; }

  bool operator<=(const int64_t &other) const {
    return cmp_64bits(*this, other) <= 0;
  }

  bool operator>=(const Integer &other) const { return cmp(*this, other) >= 0; }

  bool operator>=(const int64_t &other) const {
    return cmp_64bits(*this, other) >= 0;
  }

  bool operator!=(const Integer &other) const { return cmp(*this, other) != 0; }

  bool operator!=(const int64_t &other) const {
    return cmp_64bits(*this, other) != 0;
  }

  Integer operator++(int) = delete;
  Integer operator--(int) = delete;

  Integer operator+(const Integer &other) const {
    Integer out;
    add(*this, other, out);
    return out;
  }

  Integer operator+(const int64_t other) const {
    Integer out;
    add_64bits(*this, other, out);
    return out;
  }

  Integer &operator+=(const Integer &other) {
    add(*this, other, *this);
    return *this;
  }

  Integer &operator+=(const int64_t other) {
    add_64bits(*this, other, *this);
    return *this;
  }

  Integer &operator++() {
    add_64bits(*this, 1, *this);
    return *this;
  }

  Integer operator-() const {
    Integer out;
    opp(*this, out);
    return out;
  }

  Integer operator-(const Integer &other) const {
    Integer out;
    sub(*this, other, out);
    return out;
  }

  Integer operator-(const int64_t other) const {
    Integer out;
    sub_64bits(*this, other, out);
    return out;
  }

  Integer &operator-=(const Integer &other) {
    sub(*this, other, *this);
    return *this;
  }

  Integer &operator-=(const int64_t other) {
    sub_64bits(*this, other, *this);
    return *this;
  }

  Integer &operator--() {
    sub_64bits(*this, 1, *this);
    return *this;
  }

  Integer operator*(const Integer &other) const {
    Integer out;
    mul(*this, other, out);
    return out;
  }

  Integer operator*(const int64_t other) const {
    Integer out;
    mul_64bits(*this, other, out);
    return out;
  }

  Integer &operator*=(const Integer &other) {
    mul(*this, other, *this);
    return *this;
  }

  Integer &operator*=(const int64_t other) {
    mul_64bits(*this, other, *this);
    return *this;
  }

  Integer operator/(const Integer &other) const {
    Integer out;
    div(*this, other, out);
    return out;
  }

  Integer operator/(const int64_t other) const {
    Integer out;
    div_64bits(*this, other, out);
    return out;
  }

  Integer &operator/=(const Integer &other) {
    div(*this, other, *this);
    return *this;
  }

  Integer &operator/=(const int64_t other) {
    div_64bits(*this, other, *this);
    return *this;
  }

  Integer operator%(const Integer &other) const {
    Integer out;
    mod(*this, other, out);
    return out;
  }

  int64_t operator%(const int64_t other) const {
    int64_t out;
    mod_64bits(*this, other, out);
    return out;
  }

  Integer &operator%=(const Integer &other) {
    mod(*this, other, *this);
    return *this;
  }

  Integer &operator%=(const int64_t other) {
    int64_t out;
    mod_64bits(*this, other, out);
    return *this = out;
  }

  Integer operator<<(const uint64_t other) const {
    Integer out;
    shl_abs(*this, other, out);
    return out;
  }

  Integer &operator<<=(const uint64_t other) {
    shl_abs(*this, other, *this);
    return *this;
  }

  Integer operator>>(const uint64_t other) const {
    Integer out;
    shr_abs(*this, other, out);
    return out;
  }

  Integer &operator>>=(const uint64_t other) {
    shr_abs(*this, other, *this);
    return *this;
  }

  friend std::istream &operator>>(std::istream &is, Integer &a);
  friend std::ostream &operator<<(std::ostream &os, const Integer &a);

  // 1: a > b; 0: a == b; -1: a < b
  static int cmp(const Integer &a, const Integer &b);
  // -a
  static void opp(const Integer &a, Integer &out);
  // a + b
  static void add(const Integer &a, const Integer &b, Integer &out);
  // a - b
  static void sub(const Integer &a, const Integer &b, Integer &out);
  // a * b
  static void mul(const Integer &a, const Integer &b, Integer &out);
  // a % b
  static void mod(const Integer &a, const Integer &b, Integer &out);
  // quot = a / b, rem = a % b
  static void div(const Integer &a, const Integer &b, Integer &quot,
                  Integer *rem = nullptr);
  // a * pow(2, b)
  static void shl_abs(const Integer &a, uint64_t b, Integer &out);
  // a / pow(2, b)
  static void shr_abs(const Integer &a, uint64_t b, Integer &out);

  static int cmp_64bits(const Integer &a, int64_t b);
  static void add_64bits(const Integer &a, int64_t b, Integer &out);
  static void sub_64bits(const Integer &a, int64_t b, Integer &out);
  static void mul_64bits(const Integer &a, int64_t b, Integer &out);
  static void mod_64bits(const Integer &a, int64_t b, int64_t &out);
  static void div_64bits(const Integer &a, int64_t b, Integer &quot,
                         int64_t *rem = nullptr);

  static Integer random(const Integer &bound);

private:
  Integer(const bool n, VecView &&v) : neg_(n),
    abs_val_(std::move(v)) {
  }

  bool neg_;
  VecView abs_val_;
};
} // namespace lll

#endif // LLL_INTEGER_H