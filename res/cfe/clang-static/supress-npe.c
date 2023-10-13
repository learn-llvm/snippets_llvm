void usePointer(int *b);

/// ignore totally scan-build
#ifndef __clang_analyzer__  
int foo1(int *b){
  if(!b){
    usePointer(b);
  }
  return *b;
}
#endif

// the check 'if' should be eliminated to suppress scan-build npe warning
int foo(int *b){
  usePointer(b);
  return *b;
}
