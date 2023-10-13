int foo(int a, int b) {
  int c = a * b;
  if (a == 5) {
    c = a * c;
  }
  return c;
}
