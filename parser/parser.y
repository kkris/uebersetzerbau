%{
    #include <stdio.h>

    #define YYDEBUG 1

    int errors = 0;

    int yylex(void);
    void yyerror(char *);
%}

/* definitions */

%start Program
%token IDENT
%token NUM
%token FUN IF THEN ELSE LET IN NOT HEAD TAIL AND END ISNUM ISLIST ISFUN ARROW

%%

/* rules */

Program: 
       | Program Def ';'
       ;

Def: IDENT '=' Lambda
   ;

Lambda: FUN IDENT ARROW Expr END
      ;

Expr: IF Expr THEN Expr ELSE Expr END
    | Lambda
    | LET IDENT '=' Expr IN Expr END
    | Term
    /*| { not | head | tail | isnum | islist | isfun } Term  */
    | Term '+' Term
    | Term '-' Term
    | Term '*' Term
    | Term '.' Term
    | Term AND Term
    | Term '<' Term
    | Term '=' Term
    | Expr Term
    ;

Term: '(' Expr ')'
    | NUM
    | IDENT
    ;

%%

/* subroutines */

void yyerror(char *s) {
    errors++;
    fprintf(stderr, "Error: %s\n", s);
}

int main(void) {
    yyparse();

    if(errors > 0)
        return 2;

    return 0;
}
