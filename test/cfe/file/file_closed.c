#include <stdio.h>

void writeCharToLog(char *data) {
  FILE *F = fopen("syslog.txt", "w");

  if (F != NULL) {
    if (!data) return;
    fputc(*data, F);
    fclose(F);
  }
  return;
}
