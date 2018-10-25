struct S1 {
  int a;
  char b;
};

int add1(struct S1 s1) { return s1.a + s1.b; }

int add2(struct S1 *s1) { return s1->a + s1->b; }

int main(void) {
  struct S1 s1;
  int c1;
  c1 = add1(s1);
  c1 = add2(&s1);
  return 0;
}
