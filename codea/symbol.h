#ifndef SYMBOL_H
#define SYMBOL_H

enum {
    SYMBOL_TYPE_FUN,
    SYMBOL_TYPE_NONE
};

struct symbol {
    char *name;
    int type;
    struct symbol *next;
};

struct symbol *symbol_new();
struct symbol *symbol_copy(struct symbol *sym);
struct symbol *symbol_find(struct symbol *sym, char *name);
struct symbol *symbol_add(struct symbol *sym, char *name, int type);
struct symbol *symbol_merge(struct symbol *s1, struct symbol *s2);

int symbol_contains(struct symbol *sym, char *name);
void check_variable(struct symbol *sym, char *name);

void symbol_print(struct symbol *sym);

#endif // SYMBOL_H
