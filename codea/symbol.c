#include "symbol.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


struct symbol *symbol_new() {
    return NULL;
}

struct symbol *symbol_copy(struct symbol *sym) {
    struct symbol *copy = symbol_new();

    struct symbol *current = sym;

    while(current != NULL) {
        copy = symbol_add(copy, current->name);

        current = current->next;
    }

    return copy;
}

struct symbol *symbol_find(struct symbol *sym, char *name) {
    struct symbol *current = sym;

    while(current != NULL) {
        if(strcmp(current->name, name) == 0)
            return current;

        current = current->next;
    }

    return NULL;
}

struct symbol *symbol_add(struct symbol *sym, char *name) {
    sym = symbol_copy(sym);
    struct symbol *result = symbol_find(sym, name);

    if(result != NULL) {
        printf("%s already defined\n", name);
        exit(3);
    }

    struct symbol *element = malloc(sizeof(struct symbol));
    element->name = strdup(name);
    element->next = sym;

    return element;
}

struct symbol *symbol_merge(struct symbol *s1, struct symbol *s2) {
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

int symbol_contains(struct symbol *sym, char *name) {
    return symbol_find(sym, name) != NULL;
}

void check_variable(struct symbol *sym, char *name) {
    if(symbol_contains(sym, name) == 0) {
        printf("%s used but undefined\n", name);
        exit(3);
    }
}

void symbol_print(struct symbol *sym) {
    if(sym == NULL) return;

    printf("%s\n", sym->name);

    symbol_print(sym->next);
}
