all: asmb

asmb:
	gcc -o asmb.o -c asmb.s

test: reference impl
	python test/test.py 1000

reference:
	gcc test/test.c test/asmb.c --shared -std=c99 -fPIC -o reference.so

impl:
	gcc test/test.c asmb.s --shared -std=c99 -fPIC -o impl.so

clean:
	rm -f reference.so impl.so asmb.o
