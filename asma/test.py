from cffi import FFI

ffi = FFI()

ffi.cdef("""
    int run_test(unsigned long x0, unsigned long x1,
                 unsigned long y0, unsigned long y1,
                 unsigned long r0, unsigned long r1);
""")

lib = ffi.dlopen("./asma_reference.so")

lib.run_test(1, 2, 3, 4, 5, 6)
