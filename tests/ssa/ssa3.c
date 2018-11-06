/// mem2reg won't compute constants
int ssa3(int z) {
  return *(&z + 1 - 1);
}
