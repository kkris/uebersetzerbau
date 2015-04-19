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
    | UnaryOps Term
    | Term PlusTerm
    | Term '-' Term
    | Term MulTerm
    | Term ListTerm
    | Term AndTerm
    | Term '<' Term
    | Term '=' Term
    | Expr Term     /* Funktionsaufruf */
    ;

UnaryOp: NOT
       | HEAD
       | TAIL
       | ISNUM
       | ISLIST
       | ISFUN
       ;

PlusTerm: /* empty */
        | PlusTerm '+' Term
        ;

MulTerm: /* empty */
       | MulTerm '*' Term
       ;

AndTerm: /* empty */
       | AndTerm AND Term
       ;

ListTerm: /* empty */
        | ListTerm '.' Term
        ;

UnaryOps: UnaryOp
        | UnaryOps UnaryOp
        ;

Term: '(' Expr ')'
    | NUM
    | IDENT         /* Variablenverwendung */
    ;

%%

/* subroutines */

extern int lineno;

void yyerror(char *s) {
    errors++;
    fprintf(stderr, "Error: %s at line %d\n", s, lineno);
}

int main(void) {
    yyparse();

    if(errors > 0)
        return 2;

    return 0;
}
