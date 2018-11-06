#include <stdio.h>

int foo(int num) {
  int sum = 0;
  for (unsigned i = 0; i < num; ++i) {
    if (i % 2) {
      i = i + 2;
    } else {
      i = i - 2;
    }
  }
  printf("good\n");
  for (unsigned i = 0; i < num; ++i) {
    sum += i;
  }
  return sum;
}
