%{
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>

    #include "scope.h"

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

@traversal @lefttoright @preorder LRpre

@attributes { char *name; } IDENT
@attributes { struct scope *scope; } Program Lambda Expr Term
/*@attributes {struct sym_table *table;} Term Expr Lambda Def Program*/
%{
    #include <stdio.h>
    struct scope *global_scope = NULL;

%}

%%

/* rules */

Program: 
       @{ @i @Program.scope@ = global_scope = scope_new(); @}
       | Program IDENT '=' Lambda ';'
       @{ @i @Lambda.scope@ = scope_add_name(NULL, @IDENT.name@);
          @i @Program.0.scope@ = global_scope = scope_add_name(global_scope, @IDENT.name@); @}
       ;

Lambda: FUN IDENT ARROW Expr END
      @{ @i @Expr.scope@ = scope_add_name(scope_new_with_parent(@Lambda.scope@), @IDENT.name@); @}
      ;

Expr: /*IF Expr THEN Expr ELSE Expr END
    | Lambda
    | LET IDENT '=' Expr IN Expr END*/
    | Term
    @{ @i @Term.scope@ = @Expr.scope@; @}
/*    | NOT Term
    | HEAD Term
    | TAIL Term
    | ISNUM Term
    | ISLIST Term
    | ISFUN Term*/
    | Term '+' Term
    @{ @i @Term.0.scope@ = /*@Term.1.scope@ =*/ @Expr.scope@; @}
/*    | Term '-' Term
    | Term '*' Term
    | Term '.' Term
    | Term AND Term
    | Term '<' Term
    | Term '=' Term
    | Expr Term     /* Funktionsaufruf *1/*/
    ;

Term: '(' Expr ')'
    @{ @i @Expr.scope@ = @Term.scope@; @}
    | NUM
    | IDENT         /* Variablenverwendung */
    @{ @LRpre check_in_scope(@Term.scope@, @IDENT.name@); @}
    ;

%%

/* subroutines */

void yyerror(char *s) {
    errors++;
    fprintf(stderr, "Error: %s\n", s);
}

int main(void) {
    yyparse();

    scope_print(global_scope);

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
