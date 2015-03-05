%{
    #include <stdio.h>
    int yylex(void);
    void yyerror(char *);
%}

/* definitions */

%start Program
%token IDENT
%token NUM
%token FUN IF THEN ELSE LET IN NOT HEAD TAIL AND END ISNUM ISLIST ISFUN

%%

/* rules */

/*Program: Def
       ;

Def: IDENT '=' Lambda
   ;

Lambda: "fun" IDENT "->" Term "end"
      ;

Expr: "if" Expr "then" Expr "else" Expr "end"
    | Lambda
    | "let" IDENT "=" Expr "in" Expr "end"
    | Term
    /*| { "not" | "head" | "tail" | "isnum" | "islist" | "isfun" } Term */
 /*   | Term { "+" Term }
    ;


Term: "(" Expr ")"
    | IDENT
    ;
*/

Program:
       | Program Def ';'
       ;

Def: IDENT '=' Lambda
   ;

Lambda: FUN IDENT "->" Expr END
      ;

Expr: IF Expr THEN Expr ELSE Expr END
    | Lambda  
    | LET IDENT '=' Expr IN Expr END
    | Term
    /*| { not | head | tail | isnum | islist | isfun } Term  */
    /*| Term { '+' Term }  */
    | Term '-' Term
    /*| Term { ’*’ Term }  */
    /*| Term { and Term }  */
    /*| Term { ’.’ Term }  */
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
    fprintf(stderr, "Error: %s\n", s);
}

int main(void) {
    yyparse();

    return 0;
}
