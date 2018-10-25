typedef struct ST {
  int ST_a;
  int *ST_b;
} ST;

typedef struct SST {
  int SST_a;
  ST SST_st;
} SST;

int foo(void) {
  int i[10];
  int j = i[3];

  ST st;
  st.ST_a = 4;
  st.ST_b = &i[100];

  SST sst;
  sst.SST_st.ST_a = 9;

  return 0;
}
