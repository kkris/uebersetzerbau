all: scanner

scanner:
	flex scanner.l
	gcc *.c -lfl -o scanner

test: clean scanner
	./scanner

clean:
	rm -f scanner lex.yy.c
