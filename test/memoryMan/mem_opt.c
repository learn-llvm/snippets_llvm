#include <stdio.h>
#include <string.h>

struct {
  char name[40];
  int age;
} person, person_copy;

int main() {

  char str1[] = "memmove can be very useful......";
  memmove(str1 + 20, str1 + 15, 11);
  puts(str1);

  char str2[] = "almost every programmer should know memset!";
  memset(str2, '-', 6);
  puts(str2);

  char myname[] = "Pierre de Fermat";

  memcpy(person.name, myname, strlen(myname) + 1);
  person.age = 46;

  memcpy(&person_copy, &person, sizeof(person));
  printf("person_copy: %s, %d \n", person_copy.name, person_copy.age);

  return 0;
}
