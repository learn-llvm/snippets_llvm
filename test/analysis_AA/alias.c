int use(int i, float f) {
  if (i < 0) {
    if (f < 0) {
      return 0;
    } else {
      return 1;
    }
  } else {
    if (f < 0) {
      return 2;
    } else {
      return 3;
    }
  }
}

int foo(int *x, float *y) {
  *x = 1;
  int i = *x;
  *y = 1.0f;
  float f = *y;
  return use(i, f);
}


int main(void) {
  float f = 0.5;
  int i = 2;
  foo(&f, &f);
  foo(&i, &f);
  return 0;
}
