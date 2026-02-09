#include "math.hpp"

namespace lll {
void pow(const Integer &a, uint64_t b, Integer &out) {
  if (b == 0) {
    out = 1;
    return;
  }

  Integer c = a;
  out = 1;
  while (true) {
    if (b & 1) out *= c;
    b /= 2;
    if (b == 0) break;
    c *= c;
  }
}

void gcd(const Integer &a, const Integer &b, Integer &out) {
  Integer ca = a.abs(), cb = b.abs(), c;
  Integer *pa = &ca, *pb = &cb, *pc = &c;

  while (true) {
    Integer::mod(*pa, *pb, *pc);
    if (pc->zero()) {
      out = *pb;
      return;
    }

    Integer *pa_ = pa;
    pa = pb;
    pb = pc;
    pc = pa_;
  }
}

void sqrt(const Integer &n, Integer &out) {
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
  out = x;
}

bool is_prime(const Integer &n) {
  if (n < 2) return false;

  Integer rem = n % 2;
  if (rem.zero()) return true;

  Integer m(3);
  Integer sqrt_n;
  sqrt(n, sqrt_n);
  while (m <= sqrt_n) {
    Integer::mod(n, m, rem);
    if (rem.zero()) return false;
    m += 2;
  }
  return true;
}
} // namespace lll