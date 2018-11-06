int foo(int n){
    int i = n;
    int sum = 0;
    while(1){
        if(i>=n) break;
        sum += i;
        ++i;
    }
    return sum;
}
