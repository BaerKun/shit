#include "math.hpp"
#include <stdexcept>

namespace lll {
static inline void swap(Integer *&px, Integer *&py) noexcept {
  Integer *pt = px;
  px = py;
  py = pt;
}

Integer pow(const Integer &b, uint64_t e) {
  Integer out = 1;
  if (e == 0) return out;

  Integer c = b;
  while (true) {
    if (e & 1) out *= c;
    e /= 2;
    if (e == 0) return out;
    c *= c;
  }
}

Integer pow_mod(const Integer &b, uint64_t e, const Integer &m) {
  if (m.zero()) throw std::domain_error("m = 0.");
  if (m == 1) return 0;
  if (e == 0) return 1;

  Integer out = 1;
  Integer c = b % m;
  while (true) {
    if (e & 1) {
      out *= c;
      out %= m;
    }
    e /= 2;
    if (e == 0) return out;
    c *= c;
    c %= m;
  }
}


Integer gcd(const Integer &a, const Integer &b) {
  Integer ca = a.abs(), cb = b.abs(), c;
  Integer *pa = &ca, *pb = &cb, *pc = &c;

  while (true) {
    Integer::mod(*pa, *pb, *pc);
    if (pc->zero()) return std::move(*pb);

    // a, b, c = b, c, a;
    swap(pa, pb);
    swap(pb, pc);
  }
}

Integer sqrt(const Integer &n) {
  if (n.neg()) throw std::domain_error("n < 0.");

  Integer x = n;
  Integer y = n + 1;
  y /= 2;

  Integer *px = &x, *py = &y;
  while (*px > *py) {
    // x = y;
    swap(px, py);

    // y = (x + n / x) // 2;
    Integer::div(n, *px, *py);
    *py += *px;
    *py /= 2;
  }
  return std::move(*px);
}

bool is_prime(const Integer &n) {
  if (n < 2) return false;

  Integer rem = n % 2;
  if (rem.zero()) return false;

  const Integer sqrt_n = sqrt(n);
  for (Integer m = 3; m <= sqrt_n; m += 2) {
    Integer::mod(n, m, rem);
    if (rem.zero()) return false;
  }
  return true;
}
} // namespace lll