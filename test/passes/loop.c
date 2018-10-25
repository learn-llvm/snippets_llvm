int bar(int x) {
  return x + 42;
}

void foo(int x, int * in, int * out, int N){
  for(int i = 0; i < N; ++i){
    int lic = x * bar(x);
    out[i] = in[i] + lic;
  }
}
