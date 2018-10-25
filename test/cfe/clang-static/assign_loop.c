int findChars(char* str, char c){
  int n = 0, i= 0;
  char a;
  while(a = str[i++]){
    if(a==c){
      n++;
    }
  }
  return n;
}
