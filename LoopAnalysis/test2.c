// test2.c
int main() {
  int i;
  int A[40], C[40], D[40];
  for (i = 2; i < 20; i++) {
    A[i] = C[i];
    D[i] = A[3 * i - 4];
    D[i - 1] = C[2 * i];
  }
  return 0;
}
