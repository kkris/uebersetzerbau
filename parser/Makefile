all: parser

yacc:
	yacc -d parser.y

flex: yacc
	flex scanner.l

parser: yacc flex
	gcc -o parser lex.yy.c y.tab.c -lfl

test: parser
	./parser

tests: parser
	../run_tests.sh ../tests/ ./parser


clean:
	rm -f parser lex.yy.c y.tab.c y.tab.h
