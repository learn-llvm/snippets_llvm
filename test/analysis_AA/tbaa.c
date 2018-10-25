void foo(int n, double *p, int *q) {
  int i;
  for (i = 0; i < n; i++) *p += *q;
}
