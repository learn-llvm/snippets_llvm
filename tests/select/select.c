int foo(int i, int j) {
  if (i < 0) {
    if (j < 0) {
      return i * j;
    } else {
      return -i * j;
    }
  } else if (i == 0) {
    return 0;
  } else {
    return i +j;
  }
}
