#include <math.h>
/// opt -mem2reg 

int bar();

int foo() {
  int y, z;
  y = bar();
  if (y < 9) {
    z = y + 1;
  } else {
    z = y * 2;
  }
  return z;
}
