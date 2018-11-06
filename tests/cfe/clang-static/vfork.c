#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
int main(void) {
  pid_t pid = vfork();
  if(pid==0){
    exit(0);
  }
  return 0;
}
