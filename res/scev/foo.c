void foo(char a[100][100], char b[100][100], int p, int k) {
    int i,j;
    for (i = 0; i < 100; i++) {
        for (j = 0; j < 10; j++) {
            a[i][p*j+k] = b[i][j*j];
        }
    }
}
