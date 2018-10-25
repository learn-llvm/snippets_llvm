int foo(int &i) { i = 3; }

int foo(int *const i) { *i = 3; }
