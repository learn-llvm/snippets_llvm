int g = 0;
static int static_g = 1;
extern int extern_g;

int main(void) {
  g = extern_g;
  return 0;
}
