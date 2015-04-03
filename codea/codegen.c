#include "codegen.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

static char *type_checked_vars[5] = {NULL, NULL, NULL, NULL, NULL};

char *get_next_reg(const char *prev) {
    fprintf(stderr, "next_reg(%s, ", prev);
    if(prev == NULL)
        return strdup("rdi");

    char* registers[] = {"rdi", "rax", "rsi", "rdx", "rcx", "r8", "r9", "r10"};

    int i = 0;
    for(; i < sizeof(registers) / sizeof(registers[0]) - 1; i++) {
        if(strcmp(prev, registers[i]) == 0) {
            fprintf(stderr, "%s)\n", registers[i + 1]);
            return strdup(registers[i + 1]);
        }
    }

    fprintf(stderr, "Upps, no register left. Implement a better allocation algorithm!\n");

    return NULL;
}

void gen_code(const char *code, ...)
{
    va_list ap;

    va_start(ap, code);
    fprintf(stdout, "\t");
    vfprintf(stdout, code, ap);
    fprintf(stdout, "\n");
    va_end(ap);
}

void gen_func(const char *name)
{
    printf(".globl %s\n%s:\n", name, name);
}

void move(const char *source, const char *dest)
{
    if(strcmp(source, dest) == 0)
        return; // nop

    gen_code("movq %%%s, %%%s", source, dest);
}

void expect(const char *var_reg, int type)
{
    int i;
    for(i = 0; i < 5; i++) {
        if(type_checked_vars[i] == NULL) break;
        if(strcmp(var_reg, type_checked_vars[i]) == 0)
            return;
    }

    if(type == TYPE_NUMBER) {
        gen_code("bt %%%s, 0", var_reg);
        gen_code("jc .raisesig");

        type_checked_vars[i] = strdup(var_reg);
    }
}

void tag(int type, const char *source, const char *dest)
{
    if(type == TYPE_NUMBER) {
        gen_code("salq %%%s, %%%s", source, dest);
    } else {
        printf("Not implemented\n");
    }
}

long int tag_const(long int value)
{
    return value << 1;
}

void untag(const char *source, const char *dest)
{
    gen_code("sarq %%%s, %%%s", source, dest);
}

void load_tagged_num(const char *var_reg, const char *dest)
{
    expect(var_reg, TYPE_NUMBER);
    move(var_reg, dest);
}

void load_num(const char *var_reg, const char *dest)
{
    expect(var_reg, TYPE_NUMBER);
    gen_code("sarq %%%s, %%%s", var_reg, dest);
}

void ret(struct tree *node, int tag_type, int type)
{
    if(node->op == OP_VAR) {
        move(node->var_reg, "rax");
    } else if(tag_type == TAGGED) {
        move(node->reg, "rax");
    } else {
        if(type == TYPE_NUMBER) {
            long int tagged = node->value << 1;
            gen_code("movq $%ld, %rax", tagged);
        } else if(type == TYPE_REG) {
            move(node->reg, "rax");
            tag(TYPE_NUMBER, "rax", "rax");
        } else {
            gen_code("ret todo");
        }
    }

    gen_code("ret");

    int i;
    for(i = 0; i < 5; i++) {
        type_checked_vars[i] = NULL;
    }
}

void gen_not(const char *source, const char *dest, int tag_type)
{
    move(source, dest);

    if(tag_type == TAGGED) {
        gen_code("xorq $2, %%%s", dest);
    } else {
        gen_code("xorq $1, %%%s", dest);
    }
}

static void gen_add_reg_const(long int value, const char *source, const char *dest, int tag_type)
{
    if(tag_type == TAGGED)
        value = tag_const(value);

    move(source, dest);
    gen_code("addq $%ld, %%%s", value, dest);
}

void gen_add(struct tree *node, int tag_type)
{
    struct tree *lhs = LEFT_CHILD(node);
    struct tree *rhs = RIGHT_CHILD(node);

    const char *dest = node->reg;

    if(lhs->constant && rhs->constant) {
        gen_code("TODO: upps, constant fold!");
    } else if(lhs->constant) {
        gen_add_reg_const(lhs->value, rhs->reg, dest, tag_type);
    } else if(rhs->constant) {
        gen_add_reg_const(rhs->value, lhs->reg, dest, tag_type);
    } else if(lhs->op == OP_VAR && rhs->op == OP_VAR){
        expect(lhs->var_reg, TYPE_NUMBER);
        expect(rhs->var_reg, TYPE_NUMBER);
        move(lhs->var_reg, dest);
        gen_code("addq %%%s, %%%s", rhs->var_reg, dest);
    } else {
        move(lhs->reg, dest);
        gen_code("addq %%%s, %%%s", rhs->reg, dest);
    }
}

static void gen_mul_reg_const(long int value, const char *source, const char *dest)
{
    move(source, dest);
    gen_code("mulq $%ld, %%%s", value, dest);
}

void gen_mul(struct tree *node)
{
    struct tree *lhs = LEFT_CHILD(node);
    struct tree *rhs = RIGHT_CHILD(node);

    const char *dest = node->reg;

    if(lhs->constant && rhs->constant) {
        gen_code("TODO: upps, constant fold!");
    } else if(lhs->constant) {
        gen_mul_reg_const(lhs->value, rhs->reg, dest);
    } else if(rhs->constant) {
        gen_mul_reg_const(rhs->value, lhs->reg, dest);
    } else {
        move(lhs->reg, dest);
        gen_code("mulq %%%s, %%%s", rhs->reg, dest);
    }
}

static void gen_eq_tagged_const(long int value, const char *source, const char *dest)
{
    value = tag_const(value);

    gen_code("xorq %%%s, %%%s", dest);
    gen_code("testq $%ld, %%%s", value, source);
    gen_code("cmovqe $2, %%%s", dest);
}

void gen_eq_tagged(struct tree *node)
{
    struct tree *lhs = LEFT_CHILD(node);
    struct tree *rhs = RIGHT_CHILD(node);

    const char *dest = node->reg;

    if(lhs->constant && rhs->constant) {
        gen_code("TODO: upps, constant fold!");
    } else if(lhs->constant) {
        if(rhs->op == OP_VAR)
            gen_eq_tagged_const(lhs->value, rhs->var_reg, dest);
        else
            gen_eq_tagged_const(lhs->value, rhs->reg, dest);
    } else if(rhs->constant) {
        if(lhs->op == OP_VAR)
            gen_eq_tagged_const(rhs->value, lhs->var_reg, dest);
        else
            gen_eq_tagged_const(rhs->value, lhs->reg, dest);
    } else if(lhs->op == OP_VAR && rhs->op == OP_VAR) {
        if(strcmp(lhs->name, rhs->name) == 0) {
            gen_code("movq $2, %%%s", dest);
        } else {
            gen_code("xorq %%%s, %%%s", dest, dest);
            gen_code("testq %%%s, %%%s", lhs->var_reg, rhs->var_reg);
            gen_code("cmovqe $2, %%%s", dest);

        }
    } else {
        gen_code("xorq %%%s, %%%s", dest, dest);
        gen_code("testq %%%s, %%%s", lhs->reg, rhs->reg);
        gen_code("cmovqe $2, %%%s", dest);
    }
}

void gen_eq_untagged(struct tree *node)
{
    gen_eq_tagged(node);
}

void gen_isnum(struct tree *node)
{
    struct tree *lhs = LEFT_CHILD(node);
    const char *dest = node->reg;

    if(lhs->op == OP_VAR) {
        move(lhs->var_reg, dest);
        gen_code("xorq $1, %rax");
    } else {
        gen_code("UNSUPPORTED");
    }
}
