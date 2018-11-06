#include <stdlib.h>
#include <stdio.h>

int main(void) {
  int operand1, operand2, sum, accumulator;

  operand1 = rand();
  operand2 = rand();

  __asm__(
      "movl %1, %0\n\t"
      "addl %2, %0"
      : "=r"(sum)                    /* output operands */
      : "r"(operand1), "r"(operand2) /* input operands */
      : "0");                        /* clobbered operands */

  accumulator = sum;

  __asm__(
      "addl %1, %0\n\t"
      "addl %2, %0"
      : "=r"(accumulator)
      : "0"(accumulator), "g"(operand1), "r"(operand2)
      : "0");

  printf("operand1=%d, operand2=%d, sum=%d, accumulator=%d", operand1, operand2,
         sum, accumulator);

  return accumulator;
}
