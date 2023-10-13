#include <stdio.h>
int getV(int i) { return i * 2; }
int main() {
  int num = 10;
  int sum = 0;
  int i;

  for (i = 0; i < num; i++) {
    sum += getV(i);
  }
  return 0;
}
