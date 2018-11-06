// RUN: clang -cc1 -analyze -analyzer-checker=alpha.unix.Stream

#include <stdio.h>

int main(int argc, char** argv) {
  FILE* f = fopen("file.txt", "w");
  if (argc > 5) {
    fputc('c', f);
    fclose(f);
  } else {
    return 0;
  }
  fclose(f);
  return 0;
}
