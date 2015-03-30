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

void expect(const char *reg, int type);

void tag(int type, const char *source, const char *dest);
void untag(const char *source, const char *dest);

void load_tagged_num(const char *var_reg, const char *dest);
void load_num(const char *var_reg, const char *dest);

void ret(struct tree *node, int tag_type, int type);

void gen_not(const char *source, const char *dest, int tag_type);
void gen_eq(const char *src1, const char *src2, const char *dest);
void gen_eq_with_const(const char *src1, long int value, const char *dest);

#endif // CODEGEN_H
