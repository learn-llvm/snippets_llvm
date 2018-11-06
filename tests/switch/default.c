int main(void) { return 0; }

int foo(int i) {
  int res;
  switch (i) {
    case 1:
      res = 1;
      break;
    case 2:
      res = 2;
      break;
    case 3:
      res = 3;
      break;
    default:
      res = 0;
      break;
  }
  return res;
}
