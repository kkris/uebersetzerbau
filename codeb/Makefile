all: codeb

ox:
	ox parser.y scanner.l

yacc: ox
	yacc -d oxout.y

flex: yacc
	flex oxout.l

codegen:
	bfe codegen.bfe | iburg - code.c

codeb: flex codegen
	gcc -g -o codeb lex.yy.c y.tab.c code.c codegen.c symbol.c tree.c -lfl

tests: codeb
	../testframework/run_test_call.sh ../tests ./codeb

checkcode: codeb
	../testframework/run_code_check.sh ../tests ./codeb


clean:
	rm -f codeb lex.yy.c oxout.l oxout.y y.tab.c y.tab.h code.c
