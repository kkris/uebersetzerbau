all: codea

codea: generator codegen
	gcc -g -o codea lex.yy.c y.tab.c code.c codegen.c symbol.c tree.c -lfl

generator: clean
	ox parser.y scanner.l
	yacc -d oxout.y
	flex oxout.l

codegen:
	bfe codegen.bfe | iburg - code.c


tests: codea
	../testframework/run_test_call.sh ../tests ./codea

checkcode: codea
	../testframework/run_code_check.sh ../tests ./codea


clean:
	rm -f codea lex.yy.c oxout.l oxout.y y.tab.c y.tab.h code.c
