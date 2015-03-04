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
    unsigned long x[2] = {1, 2};
    unsigned long y[2] = {3, 4};
    unsigned long r[3];

    asmb(x, y, r, 2);

    return 0;
}
