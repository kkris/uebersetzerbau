%{
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>

    #include "symbol.h"

    #define YYDEBUG 1

    int errors = 0;

    int yylex(void);
    void yyerror(char *);
%}

/* definitions */

%start Start
%token IDENT
%token NUM
%token FUN IF THEN ELSE LET IN NOT HEAD TAIL AND END ISNUM ISLIST ISFUN ARROW


@traversal @postorder verify
@autoinh symbols

@attributes { char *name; } IDENT

@attributes { struct symbol *symbols; struct symbol *defs; } Program Def
@attributes { struct symbol *symbols; } Lambda Expr Term PlusTerm MulTerm ListTerm AndTerm


%%

/* rules */

Start:  Program
     @{ @i @Program.symbols@ = @Program.defs@; @}
     ;

Program: /*empty */
       @{ @i @Program.defs@ = NULL; @}
       |
       IDENT '=' Lambda ';' Program
       @{ @i @Program.0.defs@ = symbol_add(@Program.1.defs@, @IDENT.name@);
          @i @Program.1.symbols@ = @Program.0.symbols@;
          @i @Lambda.symbols@ = @Program.0.symbols@;
       @}
       ;

Lambda: FUN IDENT ARROW Expr END
      @{ @i @Expr.symbols@ = symbol_add(@Lambda.symbols@, @IDENT.name@); @}
      ;

Expr: IF Expr THEN Expr ELSE Expr END
    | Lambda
    | LET IDENT '=' Expr IN Expr END
    @{ @i @Expr.2.symbols@ = symbol_add(@Expr.0.symbols@, @IDENT.name@); @}
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

UnaryOps: UnaryOp
        | UnaryOps UnaryOp
        ;


PlusTerm: '+' Term
        | PlusTerm '+' Term
        ;

MulTerm: '*' Term
       | MulTerm '*' Term
       ;

AndTerm: AND Term
       | AndTerm AND Term
       ;

ListTerm: '.' Term
        | ListTerm '.' Term


Term: '(' Expr ')'
    | NUM
    | IDENT         /* Variablenverwendung */
    @{ @verify check_variable(@Term.symbols@, @IDENT.name@); @}
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
