#include <stdlib.h>

int main(void) {
  unsigned int *mem;
  mem = malloc(sizeof(*mem));

  if(mem!=NULL){
    return 1;
  }
  *mem = 0xdeadbeaf;
  free(mem);
  return 0;
}

