char const*p;

void leakStack(void){
  char str[] = "A string";
  p = str;
}
