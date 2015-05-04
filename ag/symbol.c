#include "symbol.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


struct symbol *symbol_new()
{
    struct symbol *sym = malloc(sizeof(struct symbol));
    sym->name = NULL;
    sym->next = NULL;

    return sym;
}

struct symbol *symbol_copy(struct symbol *sym)
{
    if(sym == NULL)
        return NULL;

    struct symbol *copy;
    struct symbol *prev = NULL;
    struct symbol *current = sym;

    while(current != NULL) {
        copy = symbol_new();

        if(current->name != NULL)
            copy->name = strdup(current->name);

        copy->next = prev;
        prev = copy;

        current = current->next;
    }

    return copy;
}

struct symbol *symbol_find(struct symbol *sym, char *name)
{
    struct symbol *current = sym;

    while(current != NULL) {
        if(strcmp(current->name, name) == 0)
            return current;

        current = current->next;
    }

    return NULL;
}

struct symbol *symbol_add(struct symbol *sym, char *name)
{
    sym = symbol_copy(sym);
    struct symbol *result = symbol_find(sym, name);

    if(result != NULL) {
        printf("%s already defined\n", name);
        exit(3);
    }

    struct symbol *element = symbol_new();
    element->name = strdup(name);
    element->next = sym;

    return element;
}

struct symbol *symbol_merge(struct symbol *s1, struct symbol *s2)
{
    struct symbol *current = s1;
    struct symbol *result = symbol_new();

    while(current != NULL) {
        result = symbol_add(result, current->name);
        current = current->next;
    }

    current = s2;
    while(current != NULL) {
        result = symbol_add(result, current->name);
        current = current->next;
    }

    return result;
}

int symbol_contains(struct symbol *sym, char *name)
{
    return symbol_find(sym, name) != NULL;
}

void check_variable(struct symbol *sym, char *name)
{
    if(symbol_contains(sym, name) == 0) {
        printf("%s used but undefined\n", name);
        exit(3);
    }
}

void symbol_print(struct symbol *sym) 
{
    if(sym == NULL) return;

    printf("%s\n", sym->name);

    symbol_print(sym->next);
}
