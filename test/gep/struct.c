struct RT {
  char A;
  int B[10][20];
};
struct ST {
  int X;
  struct RT Z;
};

int foo(struct ST *s) { return s[1].Z.B[5][13]; }
