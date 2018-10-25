int gcd(int a, int b) {
  int c = a;
  int d = b;
  if (c == 0) return d;
  while (d != 0) {
    if (c > d)
      c = c - d;
    else
      d = d - c;
  }
  return c;
}

int main(void) {
  gcd(8, 6);
  return 0;
}
