int bar1(int n);
int bar2(int n);
int inc(int num);

int foo(int i, int n){
  while(i < bar1(n)){
    i=bar2(i);
    for(unsigned ii = 0; ii < bar2(i); inc(ii) ){
      if(bar2(ii)<n)return ii;
    }
    if(bar2(i)<n)return i;
  }
  return n;
}

