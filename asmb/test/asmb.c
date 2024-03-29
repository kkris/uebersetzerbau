#include <stddef.h>  
#include <stdio.h>
/* x, y haben n Elemente, sum hat n+1 Elemente */

void asmb(unsigned long x[], unsigned long y[],
          unsigned long r[], size_t n)
{
  unsigned long borrow, d;
  size_t i;

  borrow = 0;

  for (i=0; i<n; i++) {
    d = x[i] - y[i] - borrow;
    borrow = borrow ? d >= x[i] : d > x[i];
    r[i] = d;
  }
  r[i] = -borrow;
}
