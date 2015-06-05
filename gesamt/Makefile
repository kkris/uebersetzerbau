all: gesamt

ox:
	ox parser.y scanner.l

yacc: ox
	yacc -d oxout.y

flex: yacc
	flex oxout.l

codegen:
	bfe codegen.bfe | iburg - code.c

gesamt: flex codegen
	gcc -g -o gesamt lex.yy.c y.tab.c code.c codegen.c symbol.c tree.c -lfl

tests: gesamt
	../testframework/run_test_call.sh ../tests ./gesamt

checkcode: gesamt
	../testframework/run_code_check.sh ../tests ./gesamt


clean:
	rm -f gesamt lex.yy.c oxout.l oxout.y y.tab.c y.tab.h code.c
