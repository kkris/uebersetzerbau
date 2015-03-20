import sys
import random
from cffi import FFI

ffi = FFI()

ffi.cdef("""
    void run_test(unsigned long x[], unsigned long y[],
                 unsigned long r[], size_t n);
""")

reference_lib = ffi.dlopen("./reference.so")
implementation_lib = ffi.dlopen("./impl.so")

def zero(r, n):
    for i in range(n + 1):
        r[i] = 0


test_cases = []

MAX = 5

for _ in range(int(sys.argv[1])):
    n = random.randint(0, 10)

    x = ffi.new("unsigned long[]", n)
    y = ffi.new("unsigned long[]", n)
    r = ffi.new("unsigned long[]", n + 1)

    for i in range(n):
        x[i] = random.randint(0, MAX)
        y[i] = random.randint(0, MAX)

    test_cases.append([x, y, r, n])


for t in test_cases:
    r = t[2]
    n = t[3]

    reference_lib.run_test(t[0], t[1], r, n)

    expected = []
    for i in range(n + 1):
        expected.append(r[i])

    zero(r, n)

    implementation_lib.run_test(t[0], t[1], r, n)

    for i in range(n + 1):
        assert expected[i] == r[i]

