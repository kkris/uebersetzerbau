generator: clean
	ox parser.y scanner.l
	yacc -d oxout.y
	flex oxout.l


ag: generator
	gcc -o ag lex.yy.c y.tab.c symbol.c -lfl

debug: generator
	gcc -o ag lex.yy.c y.tab.c symbol.c -lfl -DENDEBUG


test: ag
	./ag

tests: ag
	../run_tests.sh ../tests/ ./ag


clean:
	rm -f ag lex.yy.c y.tab.c y.tab.h oxout.y oxout.l
