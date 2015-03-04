import sys
import random
from cffi import FFI

ffi = FFI()

ffi.cdef("""
    int run_test(unsigned long x0, unsigned long x1,
                 unsigned long y0, unsigned long y1,
                 unsigned long r[]);
""")

reference_lib = ffi.dlopen("./asma_reference.so")
implementation_lib = ffi.dlopen("./asma_impl.so")


test_cases = []

MAX = 2 ** 20

for _ in range(int(sys.argv[1])):
    test_cases.append([
        random.randint(0, MAX),
        random.randint(0, MAX),
        random.randint(0, MAX),
        random.randint(0, MAX)
    ])


r = ffi.new("unsigned long[]", 2)

for t in test_cases:
    reference_lib.run_test(t[0], t[1], t[2], t[3], r)

    r0 = r[0]
    r1 = r[1]

    r[0] = 0
    r[1] = 0

    implementation_lib.run_test(t[0], t[1], t[2], t[3], r)

    assert r0 == r[0] and r1 == r[1]

