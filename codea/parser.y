%{
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>

    #include "tree.h"
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
@traversal @postorder codegen

@autoinh symbols
@autosyn node

@attributes { char *name; } IDENT
@attributes { long int value; } NUM

@attributes { struct symbol *symbols; struct symbol *defs; } Program Def

@attributes { struct symbol *symbols; struct tree *node; } Term Expr Lambda


%%

/* rules */

Start:  Program
     @{ @i @Program.symbols@ = @Program.defs@; 
     @}
     ;

Program:
       @{ @i @Program.defs@ = symbol_new(); @}
       |
       IDENT '=' Lambda ';' Program
       @{ @i @Program.0.defs@ = symbol_add(@Program.1.defs@, @IDENT.name@);
          @i @Program.1.symbols@ = @Program.0.symbols@;
          @i @Lambda.symbols@ = @Program.0.symbols@;

          @codegen gen_func(@IDENT.name@);
          @codegen burm_label(@Lambda.node@); burm_reduce(@Lambda.node@, 1);
       @}
       ;

Lambda: FUN IDENT ARROW Expr END
      @{ 
        @i @Expr.symbols@ = symbol_add(@Lambda.symbols@, @IDENT.name@); 
        @i @Lambda.node@ = @Expr.node@;

      @}
      ;

Expr: /*IF Expr THEN Expr ELSE Expr END
    | Lambda
    | LET IDENT '=' Expr IN Expr END
    @{ @i @Expr.2.symbols@ = symbol_add(@Expr.0.symbols@, @IDENT.name@); @}
    |*/ Term
    /*| NOT Term
    | HEAD Term
    | TAIL Term
    | ISNUM Term
    | ISLIST Term
    | ISFUN Term*/
    | Term '+' Term
    @{
        @i @Expr.node@ = new_node(OP_ADD, @Term.0.node@, @Term.1.node@);
    @}
    /*| Term '-' Term
    | Term '*' Term
    | Term '.' Term
    | Term AND Term
    | Term '<' Term
    | Term '=' Term
    | Expr Term */    /* Funktionsaufruf */
    ;

Term: '(' Expr ')'
    @{
        @i @Term.node@ = new_node(OP_UNKNOWN, NULL, NULL);
    @}
    | NUM
    @{
        @i @Term.node@ = new_const_node(OP_NUM, NULL, NULL, @NUM.value@);
    @}
    | IDENT         /* Variablenverwendung */
    @{ 
        @i @Term.node@ = new_node(OP_UNKNOWN, NULL, NULL);
        @verify check_variable(@Term.symbols@, @IDENT.name@); 
    @}
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
