#include "math.hpp"

namespace lll {
Integer pow(const Integer &a, uint64_t b) {
  Integer out = 1;
  if (b == 0) return out;

  Integer c = a;
  while (true) {
    if (b & 1) out *= c;
    b /= 2;
    if (b == 0) return out;;
    c *= c;
  }
}

Integer gcd(const Integer &a, const Integer &b) {
  Integer ca = a.abs(), cb = b.abs(), c;
  Integer *pa = &ca, *pb = &cb, *pc = &c;

  while (true) {
    Integer::mod(*pa, *pb, *pc);
    if (pc->zero()) return std::move(*pb);

    Integer *pa_ = pa;
    pa = pb;
    pb = pc;
    pc = pa_;
  }
}

Integer sqrt(const Integer &n) {
  if (n.neg()) throw std::domain_error("n < 0.");

  Integer x = n;
  Integer y = n + 1;
  y /= 2;

  while (x > y) {
    x = y;
    Integer::div(n, x, y);
    y += x;
    y /= 2;
  }
  return x;
}

bool is_prime(const Integer &n) {
  if (n < 2) return false;

  Integer rem = n % 2;
  if (rem.zero()) return true;

  const Integer sqrt_n = sqrt(n);
  for (Integer m = 3; m <= sqrt_n; m += 2) {
    Integer::mod(n, m, rem);
    if (rem.zero()) return false;
  }
  return true;
}
} // namespace lll