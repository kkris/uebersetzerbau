parser: clean
	yacc -d parser.y
	flex parser.l
	gcc -o parser lex.yy.c y.tab.c -lfl

test: parser
	./parser

clean:
	rm -f parser lex.yy.c y.tab.c y.tab.h
