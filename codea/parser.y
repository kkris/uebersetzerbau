%{
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>

    #include "tree.h"
    #include "symbol.h"
    #include "codegen.h"

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
@traversal @preorder reg
@traversal @postorder codegen

@autoinh symbols
@autosyn node

@attributes { char *name; } IDENT
@attributes { long int value; } NUM

@attributes { struct symbol *symbols; struct symbol *defs; } Program Def

@attributes { struct symbol *symbols; struct tree *node; } Term Expr Lambda LambdaToplevel PlusTerm MulTerm ListTerm AndTerm UnaryOps

@attributes { int op; } UnaryOp


%%

/* rules */

Start:  Program
     @{ @i @Program.symbols@ = @Program.defs@; 
     @}
     ;

Program: /* empty */
       @{ @i @Program.defs@ = NULL; @}
       |
       IDENT '=' LambdaToplevel ';' Program
       @{ @i @Program.0.defs@ = symbol_add(@Program.1.defs@, @IDENT.name@, "??");
          @i @Program.1.symbols@ = @Program.0.symbols@;
          @i @LambdaToplevel.symbols@ = @Program.0.symbols@;

          /*@codegen @revorder(1) print_tree(@LambdaToplevel.node@, 0);*/
          @codegen @revorder(1) gen_func(@IDENT.name@);
          @codegen @revorder(1) burm_label(@LambdaToplevel.node@); burm_reduce(@LambdaToplevel.node@, 1);

       @}
       ;

LambdaToplevel: FUN IDENT ARROW Expr END
      @{ 
        @i @Expr.symbols@ = symbol_add(@LambdaToplevel.symbols@, @IDENT.name@, "rdi"); 
        @i @LambdaToplevel.node@ = new_node(OP_RET, @Expr.node@, NULL);

        @reg @Expr.node@->reg = "rax";

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
    @{
        @i @Expr.node@ = @Term.node@;
    @}
    | UnaryOps
    @{
        @i @Expr.node@ = @UnaryOps.node@;
        @reg @UnaryOps.node@->reg = @Expr.node@->reg;
    @}
    | Term '+' Term
    @{
        @i @Expr.node@ = new_node(OP_ADD, @Term.0.node@, @Term.1.node@);

        @reg @Term.0.node@->reg = @Expr.node@->reg;
        @reg @Term.1.node@->reg = get_next_reg(@Term.0.node@->reg, @Term.0.node@->constant);
    @}
    | Term '-' Term
    @{
        @i @Expr.node@ = new_node(OP_SUB, @Term.0.node@, @Term.1.node@);

        @reg @Term.0.node@->reg = @Expr.node@->reg;
        @reg @Term.1.node@->reg = get_next_reg(@Term.0.node@->reg, @Term.0.node@->constant);
   @}
    | Term '*' Term
    @{
        @i @Expr.node@ = new_node(OP_MUL, @Term.0.node@, @Term.1.node@);

        @reg @Term.0.node@->reg = @Expr.node@->reg;
        @reg @Term.1.node@->reg = get_next_reg(@Term.0.node@->reg, @Term.0.node@->constant);
    @}
    /*| Term '.' Term*/
    | Term AND Term
    @{
        @i @Expr.node@ = new_node(OP_AND, @Term.0.node@, @Term.1.node@);
        /*@codegen @Term.0.node@->reg = get_reg();
        @codegen @Term.1.node@->reg = get_reg();*/
    @}
    | Term '<' Term
    @{
        @i @Expr.node@ = new_node(OP_LT, @Term.0.node@, @Term.1.node@);
        /*@codegen @Term.0.node@->reg = get_reg();
        @codegen @Term.1.node@->reg = get_reg();*/
    @}
    | Term '=' Term
    @{
        @i @Expr.node@ = new_node(OP_EQ, @Term.0.node@, @Term.1.node@);

        @reg @Term.0.node@->reg = @Expr.node@->reg;
        @reg @Term.1.node@->reg = get_next_reg(@Term.0.node@->reg, @Term.0.node@->constant);
    @}
    /*| Expr Term */    /* Funktionsaufruf */
    ;

UnaryOp: NOT 
       @{ @i @UnaryOp.op@ = OP_NOT; @}
       | HEAD
       @{ @i @UnaryOp.op@ = OP_HEAD; @}
       | TAIL
       @{ @i @UnaryOp.op@ = OP_TAIL; @}
       | ISNUM
       @{ @i @UnaryOp.op@ = OP_ISNUM; @}
       | ISLIST
       @{ @i @UnaryOp.op@ = OP_ISLIST; @}
       | ISFUN
       @{ @i @UnaryOp.op@ = OP_ISFUN; @}
       ;

UnaryOps: UnaryOp Term
   @{ 
        @i @UnaryOps.node@ = new_node(@UnaryOp.op@, @Term.node@, NULL); 
        @reg @Term.node@->reg = @UnaryOps.node@->reg;
   @}
   | UnaryOp UnaryOps
   @{ 
        @i @UnaryOps.0.node@ = new_node(@UnaryOp.op@, @UnaryOps.1.node@, NULL); 
        @reg @UnaryOps.1.node@->reg = @UnaryOps.0.node@->reg;
   @}
    ;

Term: '(' Expr ')'
    @{
        @i @Term.node@ = @Expr.node@;
        @reg @Expr.node@->reg = @Term.node@->reg;
    @}
    | NUM
    @{
        @i @Term.node@ = new_const_node(OP_NUM, NULL, NULL, @NUM.value@);
    @}
    | IDENT         /* Variablenverwendung */
    @{ 
        @i @Term.node@ = new_ident_node(OP_VAR, NULL, NULL, @IDENT.name@, symbol_find(@Term.symbols@, @IDENT.name@));
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
