#include <stdio.h>
#include <assert.h>

int expect_sig = 0;
int sig_raised = 0;

int errors = 0;

static long heap[1000000];
register long *heapptr asm("%r15");


void raisesig()
{
    if(expect_sig == 0)
        printf("\tFF Did not expect a signal at this point\n");

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

#define RET(EXPR) do {\
    if(EXPR == 0) {\
        printf("\tFF expr is false but should be true\n"); \
        errors++;\
    } \
} while(0)

#define CHECK(FUNC, INPUT, EXPECTED) do {\
    long int result = untag(FUNC(tag(INPUT))); \
    if(result != EXPECTED) {\
        printf("\tFF %s(%s) == %ld but should be %s\n", #FUNC, #INPUT, result, #EXPECTED); \
        errors++;\
    } \
} while(0)

#define CHECK_RAW(FUNC, INPUT, EXPECTED) do {\
    long int result = untag(FUNC(INPUT)); \
    if(result != EXPECTED) {\
        printf("\tFF %s(%s) == %ld but should be %s\n", #FUNC, #INPUT, result, #EXPECTED); \
        errors++;\
    } \
} while(0)

#define RAISES(FUNC, INPUT) do {\
    expect_sig = 1; \
    FUNC(INPUT); \
    if(sig_raised == 0) {\
        printf("\tFF Expected signal in %s, but was not raised (input = %s)\n", #FUNC, #INPUT); \
        errors++;\
    }\
    expect_sig = 0; \
    sig_raised = 0; \
} while(0)

#define RAISES_NOT(FUNC, INPUT) do {\
    expect_sig = 0; \
    FUNC(INPUT); \
    if(sig_raised == 1) {\
        printf("\tFF Did not expect signal in %s, but was raised (input = %s)\n", #FUNC, #INPUT); \
        errors++;\
    }\
    expect_sig = 0; \
    sig_raised = 0; \
} while(0)

int main(void) {
    heapptr = heap;

    //printf("TESTCASE\n");

    #include "TESTCASE"

    if(errors == 0) {
        printf("OK TESTCASE\n");
    } else {
        printf("FF TESTCASE (%d errors)\n", errors);
    }

    return 0;
}
