int foo(int sum, int MAX) {
  for (int i = 0; i < MAX; ++i) {
    sum += i;
  }
  return sum;
}

int bar(void) {
  int sum = 0;
  for (int i = 7; i * i < 1000; ++i) {
    sum += i;
  }
  return sum;
}
