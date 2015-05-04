all: ag

ox:
	ox parser.y scanner.l

yacc: ox
	yacc -d oxout.y

flex: yacc
	flex oxout.l

ag: flex
	gcc -o ag lex.yy.c y.tab.c symbol.c -lfl

debug: flex
	gcc -o ag lex.yy.c y.tab.c symbol.c -lfl -DENDEBUG


test: ag
	./ag

tests: ag
	../testframework/run_tests.sh ../tests/ ./ag


clean:
	rm -f ag lex.yy.c y.tab.c y.tab.h oxout.y oxout.l
