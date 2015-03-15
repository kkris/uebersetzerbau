%{
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>

    #include "symtab.h"

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

@attributes {char *name;} IDENT
@attributes {struct sym_table *table;} Term Expr Lambda Def Program
%{
    struct sym_table *root_symbols;

    struct sym_table *new_from_id(char *name) {
        struct symbol *sym = malloc(sizeof(struct symbol));
        sym->name = strdup(name);
        sym->next = NULL;

        struct sym_table *table = malloc(sizeof(struct sym_table));
        table->sym = sym;
        table->parent = NULL;

        return table;
    }

    struct symbol *merge_symbols(struct symbol *s1, struct symbol *s2) {
        if(s1 == NULL)
            return s2;

        struct symbol *s = s1;

        while(s->next != NULL) {
            s = s->next;
        }

        s->next = s2;

        return s;
    }

    struct sym_table *merge(struct sym_table *s1, struct sym_table *s2) {
        struct sym_table *table = malloc(sizeof(struct sym_table));

        table->parent = NULL;
        table->sym = NULL;

        if(s1 == NULL && s2 == NULL) {

        } else if(s1 == NULL) {
            table->sym = s2->sym;
        } else if(s2 == NULL) {
            table->sym = s1->sym;
        } else {
            table->sym = merge_symbols(s1->sym, s2->sym);
        }

        return table;
    }

    void name_not_found(char *name) {
        printf("Error: %s undefined\n", name);
        exit(3);
    }

    int is_defined2(char *name, struct sym_table *table) {
        struct symbol *sym = table->sym;

        while(sym != NULL) {
            if(strcmp(sym->name, name) == 0) {
                return 1;
            }
            sym = sym->next;
        }

        struct sym_table *parent = table->parent;
        if(parent != NULL) {
            return is_defined2(name, parent);
        }

        return 0;
    }

    struct sym_table *is_defined(char *name, struct sym_table *table) {
        if(table == NULL)
            name_not_found(name);

        if(is_defined2(name, table) == 1)
            return table;

        name_not_found(name);

        return NULL;
    }

    void print_symbols(struct sym_table *table) {
        struct symbol *sym = table->sym;

        while(sym != NULL) {
            printf("%s, ", sym->name);
            sym = sym->next;
        }
        printf("\n");
    }
%}

%%

/* rules */

Program: 
       @{ @i @Program.table@ = NULL ; @}
       | Program Def ';'
       @{ @i root_symbols = @Program.table@ = merge(root_symbols, @Def.table@) ; @}
       ;

Def: IDENT '=' Lambda
   @{ @i @Def.table@ = new_from_id(@IDENT.name@) ; @}
   ;

Lambda: FUN IDENT ARROW Expr END
      @{ @i @Lambda.table@ = @Expr.table@ ; @}
      ;

Expr: IF Expr THEN Expr ELSE Expr END
    @{ @i @Expr.0.table@ = @Expr.1.table@ ; @}
    | Lambda
    @{ @i @Expr.table@ = @Lambda.table@ ; @}
    | LET IDENT '=' Expr IN Expr END
    @{ @i @Expr.0.table@ = @Expr.1.table@ ; @}
    | Term
    @{ @i @Expr.table@ = @Term.table@ ; @}
    | NOT Term
    @{ @i @Expr.table@ = @Term.table@ ; @}
    | HEAD Term
    @{ @i @Expr.table@ = @Term.table@ ; @}
    | TAIL Term
    @{ @i @Expr.table@ = @Term.table@ ; @}
    | ISNUM Term
    @{ @i @Expr.table@ = @Term.table@ ; @}
    | ISLIST Term
    @{ @i @Expr.table@ = @Term.table@ ; @}
    | ISFUN Term
    @{ @i @Expr.table@ = @Term.table@ ; @}
    | Term '+' Term
    @{ @i @Expr.table@ = merge(@Term.0.table@, @Term.1.table@); @}
    | Term '-' Term
    @{ @i @Expr.table@ = merge(@Term.0.table@, @Term.1.table@); @}
    | Term '*' Term
    @{ @i @Expr.table@ = merge(@Term.0.table@, @Term.1.table@); @}
    | Term '.' Term
    @{ @i @Expr.table@ = merge(@Term.0.table@, @Term.1.table@); @}
    | Term AND Term
    @{ @i @Expr.table@ = merge(@Term.0.table@, @Term.1.table@); @}
    | Term '<' Term
    @{ @i @Expr.table@ = merge(@Term.0.table@, @Term.1.table@); @}
    | Term '=' Term
    @{ @i @Expr.table@ = merge(@Term.0.table@, @Term.1.table@); @}
    | Expr Term     /* Funktionsaufruf */
    @{ @i @Expr.0.table@ = @Term.table@ ; @} /* TODO */
    ;

Term: '(' Expr ')'
    @{ @i @Term.table@ = @Expr.table@ ; @}
    | NUM
    @{ @i @Term.table@ = NULL; @}
    | IDENT         /* Variablenverwendung */
    @{ @i @Term.table@ = is_defined(@IDENT.name@, @Term.table@) ; @}
    ;

%%

/* subroutines */

void yyerror(char *s) {
    errors++;
    fprintf(stderr, "Error: %s\n", s);
}

int main(void) {
    yyparse();

    if(root_symbols != NULL) {
        print_symbols(root_symbols);
        /*char *name = root_symbols->sym->name;
        if(name)
            printf("End: %s\n", root_symbols->sym->name);*/
    }

    if(errors > 0)
        return 2;

    return 0;
}
