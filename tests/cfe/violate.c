typedef volatile int vint;
vint **depth;
int *b;
vint **get_depth(void) { return depth; }
int fn1(int inc) {
  int tmp = 0;
  if (get_depth() == &b) tmp = inc + **depth;
  return tmp;
}
