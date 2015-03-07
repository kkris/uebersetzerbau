#include "stdio.h"

#include "asmb.h"

void run_test(unsigned long x[], unsigned long y[],
              unsigned long r[], size_t n)
{
    asmb(x, y, r, n);

    for(int i = 0; i < n + 1; i++) {
        printf("%lu, ", r[i]);
    }

    printf("\n");
}

int main() {
    unsigned long x[4] = {1, 2, 5, 6};
    unsigned long y[4] = {3, 4, 7, 8};
    unsigned long r[5];

    run_test(x, y, r, 4);

    return 0;
}
