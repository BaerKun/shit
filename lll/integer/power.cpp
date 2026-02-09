#include "internal.hpp"

namespace lll {
using namespace internal;

void Integer::pow(const Integer &a, uint64_t b, Integer &out) {
  out = (b & 1) ? a : 1;
  b /= 2;
  if (b == 0) return;

  Integer c = a * a;
  while (true) {
    if (b & 1) out *= c;
    b /= 2;
    if (b == 0) break;
    c *= c;
  }
}
} // namespace lll