int foo(int i, int j) {
  if (i < 5) {
    while (i > 0) {
      j += i;
      --i;
    }
  }
  return j;
}
