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

Integer pow_mod(const Integer &b, const Integer &e, const Integer &m) {
  if (m.neg() || m.zero()) throw std::domain_error("m <= 0.");
  if (b.zero()) {
    if (e.neg() || e.zero()) throw std::domain_error("b == 0 && e <= 0.");
    return 0;
  }
  if (m == 1) return 0;
  if (e.zero()) return 1;
  if (e.neg()) return b == 1 ? 1 : 0;

  Integer out = 1;
  Integer c = b % m;
  const Integer::VecView &ev = e.view_v();
  for (size_t i = 0; i < ev.size(); i++) {
    uint64_t mask = ev[i];
    for (size_t j = 0; j < 64; j++) {
      // if (i == 1 && j == 61)
      if (mask & 1) {
        out *= c;
        out %= m;
      }
      mask /= 2;
      if (!mask && i == ev.size() - 1) break;
      c *= c;
      c %= m;
    }
  }
  return out;
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
  if (n < 4) return true;

  Integer rem = n % 2;
  if (rem.zero()) return false;

  const Integer sqrt_n = sqrt(n);
  for (Integer m = 3; m <= sqrt_n; m += 2) {
    Integer::mod(n, m, rem);
    if (rem.zero()) return false;
  }
  return true;
}

static bool prime_test_step(const Integer &n, const Integer &m,
                            const Integer &d, const uint64_t s) {
  const Integer a = Integer::random(n - 4) + 2;
  Integer x = pow_mod(a, d, n);
  if (x.zero()) return false;
  if (x == 1 || x == m) return true;

  for (uint64_t i = 1; i < s; i++) {
    x *= x;
    x %= n;
    if (x == m) return true;
    if (x.zero() || x == 1) return false;
  }
  return false;
}

bool prime_test(const Integer &n, size_t t) {
  if (n < 2) return false;
  if (n < 4) return true;

  const Integer m = n - 1;
  const uint64_t s = m.pow_of_2();
  if (s == 0) return false;
  const Integer d = m >> s;

  while (t--) {
    if (!prime_test_step(n, m, d, s)) return false;
  }
  return true;
}

} // namespace lll