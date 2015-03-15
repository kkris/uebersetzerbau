#include "scope.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static symbol *symbol_new_from_name(const char *name) {
    symbol *s = malloc(sizeof(symbol));
    s->name = strdup(name);

    return s;
}

static void symbol_free(symbol *sym) {
    if(sym == NULL)
        return;

    if(sym->name != NULL) {
        free(sym->name);
        sym->name = NULL;
    }

    free(sym);
}

static symbol *symbol_copy(const symbol *sym) {
    symbol *s = malloc(sizeof(symbol));

    s->name = strdup(sym->name);

    return s;
}

static symtable *symtable_new() {
    symtable *table = malloc(sizeof(symtable));

    table->count = 0;
    table->symbols = NULL;

    return table;
}

static size_t symtable_count(symtable *table) {
    if(table == NULL)
        return 0;

    return table->count;
}

static symtable *symtable_new_with_size(size_t size) {
    symtable *table = symtable_new();

    table->count = size;
    table->symbols = malloc(sizeof(symbol*) * size);

    return table;
}

static symbol *symtable_get(symtable *table, size_t index) {
    if(index >= symtable_count(table))
        return NULL;

    return table->symbols[index];
}

static void symtable_set(symtable *table, size_t index, symbol *sym) {
    if(index >= symtable_count(table))
        return;

    table->symbols[index] = sym;
}

static symtable *symtable_copy(symtable *table) {
    symtable *copy = malloc(sizeof(symtable));

    copy->count = symtable_count(table);

    size_t i;
    for(i = 0; i < copy->count; i++) {
        symbol *sym = symbol_copy(symtable_get(table, i));
        symtable_set(copy, i, sym);
    }

    return copy;
}


static void symtable_free(symtable *table) {
    if(table == NULL)
        return;

    size_t i;
    for(i = 0; i < symtable_count(table); i++) {
        symbol *s = symtable_get(table, i);
        symbol_free(s);
    }
}

static int symtable_contains(symtable *table, const char *name) {
    if(table == NULL)
        return 0;

    size_t i;
    for(i = 0; i < symtable_count(table); i++) {
        symbol *s = symtable_get(table, i);
        if(strcmp(s->name, name) == 0)
            return 1;
    }

    return 0;
}

static symtable *symtable_merge(symtable *t1, symtable *t2) {
    size_t count = symtable_count(t1) + symtable_count(t2);

    symtable *t = symtable_new_with_size(count);

    size_t i;
    size_t j = 0;
    for(i = 0; i < symtable_count(t1); i++) {
        symtable_set(t, j++, symbol_copy(symtable_get(t1, i)));
    }

    for(i = 0; i < symtable_count(t2); i++) {
        symtable_set(t, j++, symbol_copy(symtable_get(t2, i)));
    }

    return t;
}

struct scope *scope_new() {
    struct scope *s = malloc(sizeof(struct scope));

    s->parent = NULL;
    s->symtab = NULL;

    return s;
}

struct scope *scope_new_with_parent(struct scope *parent) {
    struct scope *s = malloc(sizeof(struct scope));

    s->parent = parent;
    s->symtab = NULL;

    return s;
}

int scope_contains(struct scope *s, const char *name) {
    if(s == NULL)
        return 0;

    if(symtable_contains(s->symtab, name)) {
        return 1;
    }

    return scope_contains(s->parent, name);
}

struct scope *scope_add_name(struct scope *s1, char *name) {
    struct scope *s2 = malloc(sizeof(struct scope));

    if(s1 == NULL) {
        s2->symtab = symtable_new_with_size(1);
        symtable_set(s2->symtab, 0, symbol_new_from_name(name));
        s2->parent = NULL;

        return s2;
    }

    size_t count = symtable_count(s1->symtab);
    symtable *table = symtable_new_with_size(count + 1);

    size_t i;
    for(i = 0; i < count; i++) {
        symtable_set(table, i, symbol_copy(symtable_get(s1->symtab, i)));
    }

    symtable_set(table, count, symbol_new_from_name(name));

    s2->symtab = table;
    s2->parent = s1->parent;

    return s2;
}

struct scope *scope_merge(struct scope *s1, struct scope *s2) {
    assert(s1 == NULL || s1->parent == NULL);
    assert(s2 == NULL || s2->parent == NULL);

    struct scope *s = malloc(sizeof(struct scope));

    s->parent = NULL;

    if(s1 == NULL && s2 == NULL)
        return s;
    else if(s1 == NULL)
        s->symtab = symtable_copy(s2->symtab);
    else if(s2 == NULL)
        s->symtab = symtable_copy(s1->symtab);
    else
        s->symtab = symtable_merge(s1->symtab, s2->symtab);

    return s;
}

static void scope_print_rec(struct scope *s, size_t depth) {
    if(s == NULL) 
        return;

    size_t i;

    for(i = 0; i < depth; i++) {
        printf("\t");
    }

    for(i = 0; i < symtable_count(s->symtab); i++) {
        printf("%s, ", symtable_get(s->symtab, i)->name);
    }

    printf("\n");

    scope_print_rec(s->parent, depth + 1);
}


void scope_print(struct scope *s) {
    scope_print_rec(s, 0);
}

void check_in_scope(struct scope *s, char *name) {
    if(scope_contains(s, name) == 0) {
        printf("%s used but not in scope\n", name);
        exit(3);
    }
}


