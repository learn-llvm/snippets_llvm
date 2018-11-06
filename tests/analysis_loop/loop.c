#include <stdio.h>

#define ARRAY_SIZE 100

int foo(int *a, int n) {
  int i;
  int sum = 0;
  for (; i < n; i++) {
    sum += a[i];
  }
  return sum;
}

int main() {
  int a[ARRAY_SIZE] = {1};

  int sum = foo(a, ARRAY_SIZE);

  printf("sum:0x%x\n", sum);
  return 0;
}