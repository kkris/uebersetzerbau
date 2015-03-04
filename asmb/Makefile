compile: compilereference compileimpl


compilereference:
	gcc test.c asmb.c --shared -std=c99 -fPIC -o asmb_reference.so

compileimpl:
	gcc test.c solution.s --shared -std=c99 -fPIC -o asmb_impl.so

clean:
	rm -f asmb_reference.so asmb_impl.so
