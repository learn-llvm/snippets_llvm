void customAssert() __attribute__((analyzer_noreturn));
int retNullFun(int *b){
  if(!b){
    customAssert();
  }
  return *b;
}
