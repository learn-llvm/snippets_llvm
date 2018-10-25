#include <stddef.h>
#define NUM 10
int main(void) {
  int arr[NUM];
  for(size_t i = 0; i != sizeof(arr)/sizeof(arr[0]); ++i){
    arr[i] = arr[i-1]*3;
  }
  return 0;
}
