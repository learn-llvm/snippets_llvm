///  RUN: opt < %s -basicaa -gvn -instcombine -S | grep DONOTREMOVE
int main(void) {
  int i;
  int *p = &i;
  int **pp = &p;
  int arr[3] = {1, 2, 3};
  p = arr;
  ++(*pp);
  ++p;
  i = arr[0];
  return 0;
}
