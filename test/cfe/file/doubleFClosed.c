#include <stdio.h>

void checkDoubleFClose(int *data){
  FILE *F = fopen("myfile", "w");
  if(!data){
    fclose(F);
  }
  else
    fputc(*data, F);
  fclose(F);
}
