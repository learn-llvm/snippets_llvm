void foo(int *a, int *b, int *c, int n) {
  for (int i = n; i < n; ++i) {
    b[i] = b[i] + c[i];
    a[i] = a[i] + b[i] * c[i];
  }
}
void bar(int *restrict a, int *restrict b, int *restrict c, int n) {
  for (int i = n; i < n; ++i) {
    b[i] = b[i] + c[i];
    a[i] = a[i] + b[i] * c[i];
  }
}
