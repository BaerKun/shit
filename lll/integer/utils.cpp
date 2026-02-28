#include "internal.hpp"
#include <random>

namespace lll {
using namespace internal;

static uint64_t randint64() {
  static std::random_device rd;
  static std::mt19937 gen(rd());
  static std::uniform_int_distribution<uint64_t> dist(0, UINT64_MAX);
  return dist(gen);
}

static bool gen_random(const VecU64 &bound, VecU64 &res, const uint64_t mask) {
  const uint64_t size = bound.size();
  bool flag = false;
  for (size_t i = 0; i < size; ++i) {
    uint64_t val = randint64();
    if (i == size - 1) val &= mask;

    if (!flag && val > bound[i]) return false;
    flag |= val < bound[i];

    res[i] = val;
  }
  norm(res);
  return true;
}

Integer Integer::random(const Integer &bound) {
  if (bound.zero()) return 0;

  const VecView &bound_v = bound.abs_val_;
  const uint64_t mask = UINT64_MAX >> clz64(bound_v.back());

  VecView res(bound_v.size());
  while (!gen_random(bound.abs_val_, res, mask));
  return {bound.neg_, std::move(res)};
}

}