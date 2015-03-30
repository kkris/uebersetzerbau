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

@attributes { struct symbol *symbols; struct tree *node; } Term Expr Lambda LambdaToplevel


%%

/* rules */

Start:  Program
     @{ @i @Program.symbols@ = @Program.defs@; 
     @}
     ;

Program:
       @{ @i @Program.defs@ = NULL; @}
       |
       IDENT '=' LambdaToplevel ';' Program
       @{ @i @Program.0.defs@ = symbol_add(@Program.1.defs@, @IDENT.name@, "??");
          @i @Program.1.symbols@ = @Program.0.symbols@;
          @i @LambdaToplevel.symbols@ = @Program.0.symbols@;

          @codegen gen_func(@IDENT.name@);
          @codegen burm_label(@LambdaToplevel.node@); burm_reduce(@LambdaToplevel.node@, 1);
       @}
       ;

LambdaToplevel: FUN IDENT ARROW Expr END
      @{ 
        @i @Expr.symbols@ = symbol_add(@LambdaToplevel.symbols@, @IDENT.name@, "rdi"); 
        @i @LambdaToplevel.node@ = @Expr.node@;

      @}
      ;


/*Lambda: FUN IDENT ARROW Expr END
      @{ 
        @i @Expr.symbols@ = symbol_add(@Lambda.symbols@, @IDENT.name@, "todo"); 
        @i @Lambda.node@ = @Expr.node@;

      @}
      ;
*/

Expr: /*IF Expr THEN Expr ELSE Expr END
    | Lambda
    | LET IDENT '=' Expr IN Expr END
    @{ 
        @i @Expr.2.symbols@ = symbol_add(@Expr.0.symbols@, @IDENT.name@, "todo");
        @i @Expr.0.node@ = @Expr.2.node@;
    @}
    |*/
    Term
    | NOT Term
    @{
        @i @Expr.node@ = new_node(OP_NOT, @Term.node@, NULL);
    @}
    | HEAD Term
    @{
        @i @Expr.node@ = new_node(OP_HEAD, @Term.node@, NULL);
    @}
    | TAIL Term
    @{
        @i @Expr.node@ = new_node(OP_TAIL, @Term.node@, NULL);
    @}
    | ISNUM Term
    @{
        @i @Expr.node@ = new_node(OP_ISNUM, @Term.node@, NULL);
    @}
    | ISLIST Term
    @{
        @i @Expr.node@ = new_node(OP_ISLIST, @Term.node@, NULL);
    @}
    | ISFUN Term
    @{
        @i @Expr.node@ = new_node(OP_ISFUN, @Term.node@, NULL);
    @}
    | Term '+' Term
    @{
        @i @Expr.node@ = new_node(OP_ADD, @Term.0.node@, @Term.1.node@);
    @}
    | Term '-' Term
    @{
        @i @Expr.node@ = new_node(OP_SUB, @Term.0.node@, @Term.1.node@);
    @}
    | Term '*' Term
    @{
        @i @Expr.node@ = new_node(OP_MUL, @Term.0.node@, @Term.1.node@);
    @}
    /*| Term '.' Term*/
    | Term AND Term
    @{
        @i @Expr.node@ = new_node(OP_AND, @Term.0.node@, @Term.1.node@);
    @}
    | Term '<' Term
    @{
        @i @Expr.node@ = new_node(OP_LT, @Term.0.node@, @Term.1.node@);
    @}
    | Term '=' Term
    @{
        @i @Expr.node@ = new_node(OP_EQ, @Term.0.node@, @Term.1.node@);
    @}
    /*| Expr Term */    /* Funktionsaufruf */
    ;

Term: '(' Expr ')'
    @{
        @i @Term.node@ = @Expr.node@;
    @}
    | NUM
    @{
        @i @Term.node@ = new_const_node(OP_NUM, NULL, NULL, @NUM.value@);
    @}
    | IDENT         /* Variablenverwendung */
    @{ 
        @i @Term.node@ = new_named_node(OP_VAR, NULL, NULL, @IDENT.name@, symbol_find(@Term.symbols@, @IDENT.name@)->reg);
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
