//test1.c
int main(){
    int i;
    int A[20], B[20], C[20];
    for (i = 4; i < 20; i++) {
        A[i] = C[i];
        B[i] = A[i - 4];
    }
    return 0;
}