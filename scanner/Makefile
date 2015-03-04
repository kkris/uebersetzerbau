compile:
	flex rules.lex
	gcc *.c -lfl -o scanner

test: compile
	./scanner

clean:
	rm -f scanner lex.yy.c
