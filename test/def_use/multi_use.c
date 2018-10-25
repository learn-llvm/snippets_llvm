int dumb(int A, int B, int C) {
  if (A == B)
    A = C + 5;
  else
    A = C / 5;
  return A + B;
}
