#include "stdio.h"

#include "asma.h"

void run_test(unsigned long x0, unsigned long x1,
              unsigned long y0, unsigned long y1,
              unsigned long r[])
{
    unsigned long x[2], y[2];

    x[0] = x0; x[1] = x1;
    y[0] = y0; y[1] = y1;

    asma(x, y, r);
}

int main()
{
    unsigned long x[2] = {1, 2};
    unsigned long y[2] = {3, 4};
    unsigned long r[2];

    asma(x, y, r);

    printf("[%d, %d]\n", r[0], r[1]);

    return 0;
}
