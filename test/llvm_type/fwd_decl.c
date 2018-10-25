/// the Function Type of foo is determined
int foo(int);

/// type opaque
struct ST;
int bar(struct ST *s);

/// if defined, no longer opaque
#if 0
struct ST{
  int a;
  long b;
  char *c;
  int *p;
};
#endif

struct SS {
  int a;
  char b;
  char *str;
};

/// return struct
struct SS baz(int i) {
  struct SS s = {i, 1, "good or not"};
  return s;
}

int main(int argc, char **argv) {
  foo(argc);
  struct ST *s;
  bar(s);
  return 0;
}
