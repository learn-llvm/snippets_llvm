static int global = 3;
void fun(int array[], int i) {
  array[i] = i + 1;
  global = i;
}

int main() {
  int arr[40], i = 0;
  fun(arr, 10);
  return 0;
}
