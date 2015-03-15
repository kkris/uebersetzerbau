all: asma

asma:
	gcc -c asma.s -o asma.o

reference:
	gcc test/test.c test/asma.c --shared -fPIC -o reference.so

impl:
	gcc test/test.c asma.s --shared -fPIC -o impl.so

test: impl reference
	python test/test.py 1000

clean:
	rm -f reference.so impl.so asma.o
