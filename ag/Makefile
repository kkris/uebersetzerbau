ag: clean
	ox parser.y parser.l
	yacc -d oxout.y
	flex oxout.l
	gcc -o ag lex.yy.c y.tab.c -lfl

test: ag
	./ag

tests: ag
	../run_tests.sh ../tests/ ./ag


clean:
	rm -f ag lex.yy.c y.tab.c y.tab.h oxout.y oxout.l
