#include <limits.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>

static int gcd(int a, int b) {
  while (b) {
    const int t = a % b;
    a = b;
    b = t;
  }
  return a;
}

void float2fraction_shift(const float x, int *numerator, int *shift) {
  const unsigned ix = *(unsigned *)&x;
  unsigned numerator_u = ix & 0x007fffff | 0x00800000;
  *shift = 23 - (((ix & 0x7f800000) >> 23) - 127);

  while ((numerator_u & 1) == 0) {
    numerator_u >>= 1;
    --*shift;
  }
  *numerator = x < 0.f ? -numerator_u : numerator_u;
}

void float2fraction_precise(const float x, int *numerator, int *denominator,
                            const int deno_limit) {
  const unsigned ix = *(unsigned *)&x;
  const int exponent = ((ix & 0x7f800000) >> 23) - 127;
  int limit;

  if (exponent >= 23) {
    if (exponent >= 31) {
      *numerator = INT_MAX; // too large !
      *denominator = 1;
      return;
    }
    *numerator = x;
    *denominator = 1;
    return;
  }

  if (exponent < -8) {
    if (exponent < -31) {
      *numerator = 0; // too small !
      *denominator = INT_MAX;
      return;
    }
    limit = INT_MAX;
  } else {
    limit = 1 << (23 - exponent);
  }

  if (deno_limit < limit) limit = deno_limit;

  const float abs_x = x < 0.f ? -x : x;

  int n = 0, d = 0, abs_nume = 0;
  float min_diff = INFINITY;

  while (++d < limit && min_diff != 0.f) {
    float diff1 = fabs(abs_x - (float)n / d);
    while (1) {
      const float diff2 = fabs(abs_x - (float)(n + 1) / d);
      if (diff1 < diff2) break;
      diff1 = diff2;
      ++n;
    }

    if (diff1 >= min_diff) continue;

    const float gcd_ = gcd(d, n);
    if (d / gcd_ > limit) continue;

    *denominator = d / gcd_;
    abs_nume = n / gcd_;
    min_diff = diff1;
  }
  *numerator = x < 0.f ? -abs_nume : abs_nume;
}

int main() {
  float x;
  int nume, deno;

  // 3.14159265
  printf("Enter a number: ");
  scanf("%f", &x);
  float2fraction_precise(x, &nume, &deno, 20000);
  printf("%.6f = %d / %d\n", (float)nume / deno, nume, deno);

  return 0;
}