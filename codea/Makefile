all: codea

codea: generator codegen
	gcc -o codea lex.yy.c y.tab.c code.c symbol.c node.c -lfl

generator: clean
	ox parser.y scanner.l
	yacc -d oxout.y
	flex oxout.l

codegen:
	bfe codegen.bfe | iburg - code.c


test: codea
	./codea


clean:
	rm -f codea codegen.c lex.yy.c oxout.l oxout.y y.tab.c y.tab.h
