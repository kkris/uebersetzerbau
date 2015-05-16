#ifndef CODEGEN_H
#define CODEGEN_H

#include "tree.h"

enum {
    TYPE_NUMBER = 1,
    TYPE_LIST,
    TYPE_FUN,
    TYPE_REG,
    TYPE_WORD
};

enum {
    TAGGED = 1,
    UNTAGGED
};


char *alloc_reg(const char *prev, int reuse);

void gen_func(const char *name);

void expect(const char *reg, int type);
void expect_num(struct tree *node);
void expect_list(struct tree *node);

void raise_signal();

void tag(int type, const char *source, const char *dest);
void untag(const char *source, const char *dest);

void tag_num_inplace(const char *reg);
void untag_num_inplace(const char *reg);
void tag_list_inplace(const char *reg);
void untag_list_inplace(const char *reg);

void ret(struct tree *node, int tag_type, int type);

void gen_not(struct tree *node);
void gen_and(struct tree *node);
void gen_add(struct tree *node);
void gen_sub(struct tree *node);
void gen_mul(struct tree *node);
void gen_eq(struct tree *node);
void gen_lt(struct tree *node);
void gen_isnum(struct tree *node);
void gen_islist(struct tree *node);
void gen_head(struct tree *node);
void gen_tail(struct tree *node);
void gen_list(struct tree *node);

#endif // CODEGEN_H
