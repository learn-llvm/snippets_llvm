void f(unsigned u) { unsigned short* const bad = (unsigned short*)&u; }
int main(void) {
  f(5);
  return 0;
}
