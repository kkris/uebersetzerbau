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


@traversal @lefttoright @preorder LRpre

@attributes { char *name; } IDENT

@attributes { struct symbol *symbols; struct symbol *up; } Program Def
@attributes { struct symbol *symbols; } Lambda Expr Term

/*@autoinh symbols*/

%{
    #include <stdio.h>
    struct symbol *all_symbols = NULL;
%}

%%

/* rules */

Start:  Program
     @{ @i @Program.symbols@ = all_symbols = @Program.up@; @}
     ;

Program:
       @{ @i @Program.up@ = symbol_new(); @}
       |
       IDENT '=' Lambda ';' Program
       @{ @i @Program.0.up@ = symbol_add(@Program.1.up@, @IDENT.name@);
          @i @Program.1.symbols@ = @Program.0.symbols@;
          @i @Lambda.symbols@ = @Program.0.symbols@;
       @}
       ;

/*Program: 
       @{ @i @Program.up@ = symbol_new(); @}
       | Program Def ';'
       @{ @i @Program.0.up@ = symbol_merge(@Def.up@, @Program.1.up@);
          @i @Program.1.symbols@ = @Program.0.symbols@;
       @}
       ;*/

/*Def: IDENT '=' Lambda
   @{ @i @Def.up@ = symbol_add(symbol_new(), @IDENT.name@); 
      @i @Lambda.symbols@ = @Def.symbols@;
   @}
   @{ @i @Def.symbols@ = symbol_add(symbol_new(), @IDENT.name@); 
      @i @Lambda.symbols@ = @Def.symbols@;
   @}
   ;*/

Lambda: FUN IDENT ARROW Expr END
      @{ @i @Expr.symbols@ = symbol_add(@Lambda.symbols@, @IDENT.name@); @}
      ;

Expr: /*IF Expr THEN Expr ELSE Expr END
    | Lambda
    | LET IDENT '=' Expr IN Expr END
    |*/ Term
    @{ @i @Term.symbols@ = @Expr.symbols@; @}
/*    | NOT Term
    | HEAD Term
    | TAIL Term
    | ISNUM Term
    | ISLIST Term
    | ISFUN Term*/
    /*| Term '+' Term*/
/*    | Term '-' Term
    | Term '*' Term
    | Term '.' Term
    | Term AND Term
    | Term '<' Term
    | Term '=' Term
    | Expr Term     /* Funktionsaufruf *1/*/
    ;

Term: '(' Expr ')'
    @{ @i @Expr.symbols@ = @Term.symbols@; @}
    | NUM
/*    @{ @i @Term.symbols@ = symbol_new(); @}*/
    | IDENT         /* Variablenverwendung */
/*    @{  @i @Term.symbols@ = symbol_new();
        @LRpre check_variable(@Term.symbols@, @IDENT.name@); 
    @}*/
    ;

%%

/* subroutines */

void yyerror(char *s) {
    errors++;
    fprintf(stderr, "Error: %s\n", s);
}

int main(void) {
    yyparse();

    symbol_print(all_symbols);

    /* if(root_symbols != NULL) { */
    /*     print_symbols(root_symbols); */
    /*     /*char *name = root_symbols->sym->name; */
    /*     if(name) */
    /*         printf("End: %s\n", root_symbols->sym->name);*1/ */
    /* } */

    if(errors > 0)
        return 2;

    return 0;
}
