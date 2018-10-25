#include <stdio.h>
#include <string.h>

int main(void) {
  char key[] = "apple";
  char input[80];
  do {
    printf("guess fruit?\n$ ");
    /// gets(input);
    fgets(input, sizeof(input), stdin);
  } while (strcmp(key, input) != 0);
  puts("correct!");
  return 0;
}
