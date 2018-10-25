int main(void) {
  int initialized = 3;
  int not;
  int uses_not = not;
  foo(uses_not);
  return 0;
}

int foo(int i) { return i * i; }
