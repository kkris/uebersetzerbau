#ifndef CODEGEN_H
#define CODEGEN_H

#include "tree.h"

enum {
    TYPE_NUMBER = 1,
    TYPE_LIST,
    TYPE_FUN,
    TYPE_REG
};

enum {
    TAGGED = 1,
    UNTAGGED
};


char *get_next_reg();

void gen_code(const char *code, ...);

void gen_func(const char *name);

void move(const char *source, const char *dest);

void tag(int type, const char *reg);

void untag(const char *reg);

void load_var(const char *var_reg, const char *dest);

void ret(struct tree *node, int tag_type, int type);

void gen_not(const char *source, const char *dest, int tag_type);

#endif // CODEGEN_H
