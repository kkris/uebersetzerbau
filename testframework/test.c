#include <stdio.h>
#include <assert.h>

int expect_sig = 0;
int sig_raised = 0;

void raisesig()
{
    if(expect_sig == 0)
        printf("FF Did not expect a signal at this point\n");

    sig_raised = 1;
}

long int tag(long int value) 
{
    return value << 1;
}

long int untag(long int value)
{
    return value >> 1;
}

#define CHECK(FUNC, INPUT, EXPECTED) do {\
    long int result = untag(FUNC(tag(INPUT))); \
    if(result == EXPECTED) printf("OK %s(%s) == %ld\n", #FUNC, #INPUT, result); \
   else \
        printf("FF %s(%s) == %ld but should be %s\n", #FUNC, #INPUT, result, #EXPECTED); \
} while(0)

#define RAISES(FUNC, INPUT) do {\
    expect_sig = 1; \
    FUNC(INPUT); \
    if(sig_raised == 0) printf("FF Expected signal, but was not raised\n"); \
    else printf("OK Signal raised\n"); \
    expect_sig = 0; \
    sig_raised = 0; \
} while(0)

int main(void) {
    #include "../tests/add.call"


    return 0;
}
