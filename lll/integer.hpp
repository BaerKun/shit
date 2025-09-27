#ifndef INTEGER_H
#define INTEGER_H

#include <string>
#include <vector>

namespace lll {
class Integer;

// 1: a > b; 0: a == b; -1: a < b
int cmpr(const Integer &a, const Integer &b);
void neg(const Integer &a, Integer &out);
void add(const Integer &a, const Integer &b, Integer &out);
void sub(const Integer &a, const Integer &b, Integer &out);
void mul(const Integer &a, const Integer &b, Integer &out);
void div(const Integer &a, const Integer &b, Integer &out, Integer &mod);
void pow(const Integer &a, uint64_t b, Integer &out);

class Integer {
public:
  explicit Integer(const std::string &value);
  Integer(const int64_t value = 0) // NOLINT(*-explicit-constructor)
      : data_({static_cast<uint64_t>(std::abs(value))}), neg_(value < 0) {}

  size_t size() const { return data_.size() * 8; }

  Integer &operator=(const Integer &other) = default;
  Integer &operator=(const int64_t value) {
    data_ = {static_cast<uint64_t>(std::abs(value))};
    neg_ = value < 0;
    return *this;
  }

  Integer operator+(const Integer &other) const {
    Integer out;
    add(*this, other, out);
    return out;
  }

private:
  friend int cmpr(const Integer &a, const Integer &b);
  friend void neg(const Integer &a, Integer &out);
  friend void add(const Integer &a, const Integer &b, Integer &out);
  friend void sub(const Integer &a, const Integer &b, Integer &out);
  friend void mul(const Integer &a, const Integer &b, Integer &out);
  friend void div(const Integer &a, const Integer &b, Integer &out,
                  Integer &mod);
  friend void pow(const Integer &a, uint64_t b, Integer &out);

  std::vector<uint64_t> data_; // abs
  bool neg_;
};

} // namespace lll

#endif // INTEGER_H
