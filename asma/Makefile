compile: compilereference compileimpl


compilereference:
	gcc test.c asma.c --shared -fPIC -o asma_reference.so

compileimpl:
	gcc test.c solution.s --shared -fPIC -o asma_impl.so

clean:
	rm -f asma_reference.so asma_impl.so
