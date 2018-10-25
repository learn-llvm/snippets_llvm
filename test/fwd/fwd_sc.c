int foo(int j) {
  int arr[2] = {1, 2};
  int i = arr[0] * 3;
  if (j < 0) {
    i = 1;
  } else {
    i = 2;
  }
  return i;
}

int main(void){
  foo(1);
}
