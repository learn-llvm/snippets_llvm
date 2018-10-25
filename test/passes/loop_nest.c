int foo(int i) {
  int j;
  while (i < 0) {
    ++i;
    while (j < i) {
      j += 2;
    }
  }
  return j;
}
