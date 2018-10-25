#include <unistd.h>

int main(void) {
  const char hello[] = "Hello World!\n";
  const size_t hello_size = sizeof(hello);
  ssize_t ret;
  __asm__ volatile(
      "movl $4, %%eax\n\t"
      "movl $1, %%ebx\n\t"
      "movl %1, %%ecx\n\t"
      "movl %2, %%edx\n\t"
      "int $0x80"
      : "=a"(ret)
      : "g"(hello), "g"(hello_size)
      : "%ebx", "%ecx", "%edx", "%esi", "%edi");
  return 0;
}
