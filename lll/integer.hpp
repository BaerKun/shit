#ifndef INTEGER_H
#define INTEGER_H

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

class Integer {
public:
  explicit Integer(const std::string &value);

  Integer(const int64_t value = 0) // NOLINT(*-explicit-constructor)
    : abs_val_({static_cast<uint64_t>(std::abs(value))}), neg_(value < 0) {
  }

  size_t size() const { return abs_val_.size() * 8; }

  bool zero() const { return abs_val_.size() == 1 && abs_val_[0] == 0; }

  Integer &operator=(const Integer &other) = default;

  Integer &operator=(const int64_t value) {
    abs_val_ = {static_cast<uint64_t>(std::abs(value))};
    neg_ = value < 0;
    return *this;
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

private:
  friend int cmpr(const Integer &, const Integer &);
  friend void neg(const Integer &, Integer &);
  friend void add(const Integer &, const Integer &, Integer &);
  friend void sub(const Integer &, const Integer &, Integer &);
  friend void mul(const Integer &, const Integer &, Integer &);
  friend void div(const Integer &, const Integer &, Integer &, Integer &);
  friend void pow(const Integer &, uint64_t, Integer &);

  std::vector<uint64_t> abs_val_;
  bool neg_;
};

} // namespace lll

#endif // INTEGER_H