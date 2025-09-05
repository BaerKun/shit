#include <stdio.h>

#define STEP(f, b, o)                                                          \
  array[j] = num##f o num##b;                                                  \
  if (calculate24_(array, len)) {                                              \
    printf("%d " #o " %d = %d\n", num##f, num##b, array[j]);                   \
    array[j] = numj;                                                           \
    array[i] = numi;                                                           \
    return 1;                                                                  \
  }

static int calculate24_(int array[], int len) {
  if (--len == 0 && array[0] == 24) return 1;
  for (int i = 0; i < len; i++) {
    const int numi = array[i];
    array[i] = array[len];
    for (int j = i; j < len; j++) {
      const int numj = array[j];
      STEP(i, j, +);
      STEP(i, j, *);
      if (numi > numj) {
        STEP(i, j, -);
        if (numj != 0 && numi % numj == 0) STEP(i, j, /);
      } else {
        STEP(j, i, -);
        if (numi != 0 && numj % numi == 0) STEP(j, i, /);
      }
      array[j] = numj;
    }
    array[i] = numi;
  }
  return 0;
}

void calculate24(int array[], const int len) {
  if (!calculate24_(array, len)) puts("No Solution\n");
}

int main() {
  int len;
  int number[24];

  puts("Enter the number of numbers:");
  scanf("%d", &len);
  puts("Enter the numbers:");
  for (int i = 0; i < len; i++) scanf("%d", &number[i]);
  calculate24(number, len);

  return 0;
}