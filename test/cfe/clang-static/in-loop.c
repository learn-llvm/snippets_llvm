#include <assert.h>

int foo(int length){
  int x = 0;
  assert(length > 0);
  for(int i = 0; i < length; i++){
    x +=1;
  }
  return length/x;
}
