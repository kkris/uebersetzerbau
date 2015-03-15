#ifndef SCOPE_H
#define SCOPE_H

#include <stdlib.h>
#include <string.h>

typedef struct {
    char *name;
} symbol;

typedef struct {
    size_t count;
    symbol **symbols;
} symtable;

struct scope {
    symtable *symtab;
    struct scope *parent;
};

struct scope *scope_new();
struct scope *scope_new_with_parent(struct scope *parent);
int scope_contains(struct scope *s, const char *name);
struct scope *scope_add_name(struct scope *s1, char *name);
void scope_print(struct scope *s);

void check_in_scope(struct scope *s, char *name);

#endif // SCOPE_H
