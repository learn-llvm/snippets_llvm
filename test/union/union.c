typedef union U1 {
  char ch[41];
  int b[10];
} U1;

typedef union U2 {
  struct s {
    int a;
    float f;
    double d;
  } st;

  int intArr[10];
} U2;

int main(void) {
  U1 u1a;
  U1 u1b = {{1, 2, 3, 4, 5}};
  U2 u2a = {{1, 1.0f, 1.0}};
  U2 u2b;
  return 0;
}
