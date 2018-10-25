int foo(int i){
    return i*2;
}

int main(void) {
    int x = 5;
    int y = 1;
    while (x > 1) {
        y = x * y;
        --x;
    }
    return 0;
}
