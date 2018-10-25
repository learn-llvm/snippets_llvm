#include <stdlib.h>

int main() {
  int *a, *b;
  a = (int *)malloc(sizeof(int));
  b = (int *)malloc(sizeof(int));
  *a = 10;
  *b = 8;
  return 0;
}
