#ifndef CODEGEN_H
#define CODEGEN_H

#include "tree.h"

enum {
    TYPE_NUMBER = 1,
    TYPE_LIST,
    TYPE_FUN,
    TYPE_REG
};


char *get_reg();

void gen_code(const char *code, ...);

void gen_func(const char *name);

void move(long int value, const char *reg);

void tag(int type, const char *reg);

void untag(int type, const char *reg);

void ret(int type, struct tree *node);

void gen_not(const char *source, const char *dest);

#endif // CODEGEN_H
