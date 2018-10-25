#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[]) {
  int i;
  time_t t;
  srand((unsigned)time(&t));

  for (i = 0; i < 10; ++i) {
    printf("%d\n", rand() % 50);
  }
  return 0;
}
