%{
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>

    #include "tree.h"
    #include "symbol.h"
    #include "codegen.h"

    #define YYDEBUG 1

    int errors = 0;

    int labelno = 0;
    int lambdano = 0;

    int yylex(void);
    void yyerror(char *);
%}

/* definitions */

%start Start
%token IDENT
%token NUM
%token FUN IF THEN ELSE LET IN NOT HEAD TAIL AND END ISNUM ISLIST ISFUN ARROW


@traversal @postorder verify
@traversal @preorder regalloc
@traversal @preorder codegen

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
       @{ @i @Program.0.defs@ = symbol_add(@Program.1.defs@, @IDENT.name@,
       SYMBOL_TYPE_FUN, "rax");
          @i @Program.1.symbols@ = @Program.0.symbols@;
          @i @LambdaToplevel.symbols@ = @Program.0.symbols@;

          @codegen gen_func(@IDENT.name@);
          @codegen burm_label(@LambdaToplevel.node@); burm_reduce(@LambdaToplevel.node@, 1);

       @}
       ;

LambdaToplevel: FUN IDENT ARROW Expr END
      @{ 
        @i @Expr.symbols@ = symbol_add(@LambdaToplevel.symbols@, @IDENT.name@, SYMBOL_TYPE_NONE, "rdi");
        @i @LambdaToplevel.node@ = new_node(OP_RET, @Expr.node@, NULL);

        @regalloc @Expr.node@->reg = "rax";

      @}
      ;


Lambda: FUN IDENT ARROW Expr END
      @{
        @i @Expr.symbols@ = symbol_add(@Lambda.symbols@, @IDENT.name@,
        SYMBOL_TYPE_NONE, "rdi"); 
        @i @Lambda.node@ = new_lambda_node(@Expr.node@, labelno++, @Expr.symbols@);

        @regalloc @Expr.node@->reg = "rsi";
        @regalloc @Lambda.node@->kids[0]->reg = @Expr.node@->reg;

        @regalloc mark_other_symbols_as_captured(@Lambda.node@, @IDENT.name@);

      @}
      ;


Expr: IF Expr THEN Expr ELSE Expr END
    @{
        @i @Expr.0.node@ = new_if_node(@Expr.1.node@, @Expr.2.node@, @Expr.3.node@, labelno++);

        @regalloc @Expr.2.node@->reg = @Expr.0.node@->reg;
        @regalloc @Expr.3.node@->reg = @Expr.0.node@->reg;
        @regalloc @Expr.1.node@->reg = alloc_reg(@Expr.0.node@->reg, @Expr.0.node@->constant);
        @regalloc @Expr.0.node@->kids[0]->reg = @Expr.0.node@->reg;
        @regalloc @Expr.0.node@->kids[0]->kids[0]->reg = @Expr.1.node@->reg;
    @}
    | Lambda
    @{
        @i @Expr.0.node@ = @Lambda.node@;
    @}
    | LET IDENT '=' Expr IN Expr END
    @{ 
        @i @Expr.2.symbols@ = symbol_add(@Expr.0.symbols@, @IDENT.name@, SYMBOL_TYPE_NONE, "set_later");
        @i @Expr.0.node@ = new_let_node(@Expr.1.node@, @Expr.2.node@, @Expr.2.symbols@);

        @regalloc @Expr.1.node@->reg = alloc_var_reg(@Expr.0.node@, @Expr.1.node@);
        @regalloc @Expr.2.node@->reg = alloc_reg(@Expr.1.node@->reg, @Expr.2.node@->constant);
        @regalloc @Expr.0.node@->kids[0]->reg = @Expr.1.node@->reg;

        @regalloc set_symbol_reg_children(@Expr.0.node@, @IDENT.name@, @Expr.1.node@->reg);
    @}
    |
    Term
    @{
        @i @Expr.node@ = @Term.node@;
    @}
    | UnaryOps
    @{
        @i @Expr.node@ = @UnaryOps.node@;
        @regalloc @UnaryOps.node@->reg = @Expr.node@->reg;
    @}
    | Term PlusTerm
    @{
        @i @Expr.node@ = new_node(OP_ADD, @Term.node@, @PlusTerm.node@);

        @regalloc @Term.node@->reg = @Expr.node@->reg;
        @regalloc @PlusTerm.node@->reg = alloc_reg(@Term.node@->reg, @Term.node@->constant);
    @}
    | Term '-' Term
    @{
        @i @Expr.node@ = new_node(OP_SUB, @Term.0.node@, @Term.1.node@);

        @regalloc @Term.0.node@->reg = @Expr.node@->reg;
        @regalloc @Term.1.node@->reg = alloc_reg(@Term.0.node@->reg, @Term.0.node@->constant);
   @}
    | Term MulTerm
    @{
        @i @Expr.node@ = new_node(OP_MUL, @Term.node@, @MulTerm.node@);

        @regalloc @Term.node@->reg = @Expr.node@->reg;
        @regalloc @MulTerm.node@->reg = alloc_reg(@Term.node@->reg, @Term.node@->constant);
    @}
    | Term ListTerm
    @{
        @i @Expr.node@ = new_node(OP_LIST, @Term.node@, @ListTerm.node@);

        @regalloc @Term.node@->reg = @Expr.node@->reg;
        @regalloc @ListTerm.node@->reg = alloc_reg(@Term.node@->reg, @Term.node@->constant);
    @}
    | Term AndTerm
    @{
        @i @Expr.node@ = new_node(OP_AND, @Term.node@, @AndTerm.node@);

        @regalloc @Term.node@->reg = @Expr.node@->reg;
        @regalloc @AndTerm.node@->reg = alloc_reg(@Term.node@->reg, @Term.node@->constant);
    @}
    | Term '<' Term
    @{
        @i @Expr.node@ = new_node(OP_LT, @Term.0.node@, @Term.1.node@);

        @regalloc @Term.0.node@->reg = @Expr.node@->reg;
        @regalloc @Term.1.node@->reg = alloc_reg(@Term.0.node@->reg, @Term.0.node@->constant);
    @}
    | Term '=' Term
    @{
        @i @Expr.node@ = new_node(OP_EQ, @Term.0.node@, @Term.1.node@);

        @regalloc @Term.0.node@->reg = @Expr.node@->reg;
        @regalloc @Term.1.node@->reg = alloc_reg(@Term.0.node@->reg, @Term.0.node@->constant);
    @}
    | Expr Term    /* Funktionsaufruf */
    @{
        @i @Expr.0.node@ = new_node(OP_CALL, @Expr.1.node@, @Term.node@);

        @regalloc @Expr.1.node@->reg = @Expr.0.node@->reg;
        @regalloc @Term.node@->reg = alloc_reg(@Expr.0.node@->reg, 0);
    @}
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
        @regalloc @Term.node@->reg = @UnaryOps.node@->reg;
   @}
   | UnaryOp UnaryOps
   @{ 
        @i @UnaryOps.0.node@ = new_node(@UnaryOp.op@, @UnaryOps.1.node@, NULL); 
        @regalloc @UnaryOps.1.node@->reg = @UnaryOps.0.node@->reg;
   @}
    ;

PlusTerm: '+' Term
        @{
            @i @PlusTerm.node@ = @Term.node@;
        @}
        | PlusTerm '+' Term
        @{
            @i @PlusTerm.0.node@ = new_node(OP_ADD, @PlusTerm.1.node@, @Term.node@);

            @regalloc @PlusTerm.1.node@->reg = @PlusTerm.0.node@->reg;
            @regalloc @Term.node@->reg = alloc_reg(@PlusTerm.0.node@->reg, is_const_or_atomic(@PlusTerm.0.node@));
        @}
        ;

MulTerm: '*' Term
        @{
            @i @MulTerm.node@ = @Term.node@;
        @}
        | MulTerm '*' Term
        @{
            @i @MulTerm.0.node@ = new_node(OP_MUL, @MulTerm.1.node@, @Term.node@);

            @regalloc @MulTerm.1.node@->reg = @MulTerm.0.node@->reg;
            @regalloc @Term.node@->reg = alloc_reg(@MulTerm.0.node@->reg, is_const_or_atomic(@MulTerm.0.node@));
        @}
        ;

AndTerm: AND Term
        @{
            @i @AndTerm.node@ = @Term.node@;
        @}
        | AndTerm AND Term
        @{
            @i @AndTerm.0.node@ = new_node(OP_AND, @AndTerm.1.node@, @Term.node@);

            @regalloc @AndTerm.1.node@->reg = @AndTerm.0.node@->reg;
            @regalloc @Term.node@->reg = alloc_reg(@AndTerm.0.node@->reg, is_const_or_atomic(@AndTerm.0.node@));
        @}
        ;

ListTerm: '.' Term
        @{
            @i @ListTerm.node@ = @Term.node@;
        @}
        | '.' Term ListTerm
        @{
            @i @ListTerm.0.node@ = new_node(OP_LIST, @Term.node@, @ListTerm.1.node@);

            @regalloc @Term.node@->reg = @ListTerm.0.node@->reg;
            @regalloc @ListTerm.1.node@->reg = alloc_reg(@Term.node@->reg, is_const_or_atomic(@Term.node@));
        @}

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
        @i @Term.node@ = new_ident_node(OP_VAR, NULL, NULL, @IDENT.name@, symbol_find(@Term.symbols@, @IDENT.name@));
        @regalloc @Term.node@->reg = symbol_find(@Term.symbols@, @IDENT.name@)->reg;
        @verify check_variable(@Term.symbols@, @IDENT.name@);
    @}
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
