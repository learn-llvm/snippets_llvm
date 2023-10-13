#include <stdio.h>

int main(void) {
  char* ch = "good %d";
  int i = 0;
  printf(ch, i);
  printf("perfect");
}
