#include "codegen.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>

static char *type_checked_vars[5] = {NULL, NULL, NULL, NULL, NULL};

char *get_next_reg(const char *prev, int reuse) {
    fprintf(stderr, "next_reg(%s, ", prev);
    if(prev == NULL)
        return strdup("rdi");

    char* registers[] = {"rdi", "rax", "rsi", "rdx", "rcx", "r8", "r9", "r10"};

    int i = 0;
    for(; i < sizeof(registers) / sizeof(registers[0]) - 1; i++) {
        if(strcmp(prev, registers[i]) == 0) {
            fprintf(stderr, "%s)\n", registers[i + 1]);
            if(reuse == 1)
                return strdup(registers[i]);
            else
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
        gen_code("testq $1, %%%s", var_reg);
        gen_code("jnz raisesig");

        type_checked_vars[i] = strdup(var_reg);
    }
}

void tag(int type, const char *source, const char *dest)
{
    if(type == TYPE_NUMBER) {
        move(source, dest);
        gen_code("salq $1, %%%s", dest);
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
    move(source, dest);
    gen_code("sarq $1, %%%s", dest);
}

void load_tagged_num(const char *var_reg, const char *dest)
{
    expect(var_reg, TYPE_NUMBER);
    move(var_reg, dest);
}

void load_num(const char *var_reg, const char *dest)
{
    expect(var_reg, TYPE_NUMBER);
    gen_code("xxsarq %%%s, %%%s", var_reg, dest);
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
        if(lhs->op == OP_VAR) {
            move(lhs->var_reg, dest);
            gen_code("addq %%%s, %%%s", rhs->reg, dest);
        } else if(rhs->op == OP_VAR){
            gen_code("addq %%%s, %%%s", rhs->var_reg, dest);
        } else {
            move(lhs->reg, dest);
            gen_code("addq %%%s, %%%s", rhs->reg, dest);
        }
    }
}

void gen_add_var_const(struct tree *node, int tag_type)
{
    struct tree *lhs = LEFT_CHILD(node);
    struct tree *rhs = RIGHT_CHILD(node);

    const char *dest = node->reg;

    struct tree *constnode = lhs->constant ? lhs : rhs;
    struct tree *varnode = lhs->op == OP_VAR ? lhs : rhs;

    long int value = constnode->value;
    if(tag_type == TAGGED)
        value = tag_const(value);

    expect(varnode->var_reg, TYPE_NUMBER);

    gen_code("leaq %ld(%%%s), %%%s", value, varnode->var_reg, dest);
}

static void gen_sub_reg_const(long int value, const char *source, const char *dest, int tag_type)
{
    if(tag_type == TAGGED)
        value = tag_const(value);

    move(source, dest);
    gen_code("movq $%ld, %%%s", value, dest);
    gen_code("subq %%%s, %%%s", source, dest);
}

static void gen_sub_const_reg(long int value, const char *source, const char *dest, int tag_type)
{
    if(tag_type == TAGGED)
        value = tag_const(value);

    move(source, dest);
    gen_code("subq $%ld, %%%s", value, dest);
}

void gen_sub(struct tree *node, int tag_type)
{
    struct tree *lhs = LEFT_CHILD(node);
    struct tree *rhs = RIGHT_CHILD(node);

    const char *dest = node->reg;

    if(lhs->constant && rhs->constant) {
        gen_code("TODO: upps, constant fold!");
    } else if(lhs->constant) {
        gen_sub_const_reg(lhs->value, rhs->reg, dest, tag_type);
    } else if(rhs->constant) {
        gen_sub_reg_const(rhs->value, lhs->reg, dest, tag_type);
    } else if(lhs->op == OP_VAR && rhs->op == OP_VAR){
        expect(lhs->var_reg, TYPE_NUMBER);
        expect(rhs->var_reg, TYPE_NUMBER);
        move(lhs->var_reg, dest);
        gen_code("subq %%%s, %%%s", rhs->var_reg, dest);
    } else {
        move(lhs->reg, dest);
        gen_code("subq %%%s, %%%s", rhs->reg, dest);
    }
}

/**
 * result is tagged if source reg is
 */
static void gen_mul_reg_const(const char *source, const char *dest, long int value)
{
    switch(value) {
    case 0:
    case 1:
        gen_code("mul with 0, 1 should be handled else where"); break;
    case 2:
        gen_code("leaq (, %%%s, 2), %%%s", source, dest); break;
    case 3:
        gen_code("leaq (%%%s, %%%s, 2), %%%s", source, source, dest); break;
    case 4:
        gen_code("leaq (, %%%s, 4), %%%s", source, dest); break;
    case 5:
        gen_code("leaq (%%%s, %%%s, 4), %%%s", source, source, dest); break;
    case 8:
        gen_code("leaq (, %%%s, 8), %%%s", source, dest); break;
    case 9:
        gen_code("leaq (%%%s, %%%s, 8), %%%s", source, source, dest); break;
    default:
        move(source, dest);
        gen_code("imulq $%ld, %%%s", value, dest);
    }
}

void gen_mul_untagged(struct tree *node, int input_tag_type)
{
    struct tree *lhs = LEFT_CHILD(node);
    struct tree *rhs = RIGHT_CHILD(node);

    const char *dest = node->reg;

    if(input_tag_type == TAGGED) {
        if(lhs->op == OP_VAR) {
            move(lhs->var_reg, dest);
            gen_code("sarq $1, %%%s", dest);
            gen_code("imulq %%%s, %%%s", rhs->reg, dest);
            gen_code("sarq $1, %%%s", dest);
        } else if(rhs->op == OP_VAR) {
            gen_code("sarq $1, %%%s", dest);
            gen_code("imulq %%%s, %%%s", rhs->var_reg, dest);
            gen_code("sarq $1, %%%s", dest);
        } else {
            move(lhs->reg, dest);
            gen_code("sarq $1, %%%s", dest);
            gen_code("imulq %%%s, %%%s", rhs->reg, dest);
            gen_code("sarq $1, %%%s", dest);
        }
    } else {
        if(lhs->constant) {
            gen_mul_reg_const(rhs->reg, dest, lhs->value);
        } else if(rhs->constant) {
            gen_mul_reg_const(lhs->reg, dest, rhs->value);
        } else {
            move(lhs->reg, dest);
            gen_code("imulq %%%s, %%%s", rhs->reg, dest);
        }
    }
}

void gen_mul_tagged(struct tree *node, int tag_type)
{
    struct tree *lhs = LEFT_CHILD(node);
    struct tree *rhs = RIGHT_CHILD(node);

    const char *dest = node->reg;

    assert(tag_type == TAGGED);

    if(lhs->constant && rhs->constant) {
        gen_code("TODO: upps, constant fold!");
    } else if(lhs->constant) {
        gen_mul_reg_const(rhs->reg, dest, lhs->value);
    } else if(rhs->constant) {
        gen_mul_reg_const(lhs->reg, dest, rhs->value);
    } else if(lhs->op == OP_VAR && rhs->op == OP_VAR){
        expect(lhs->var_reg, TYPE_NUMBER);
        expect(rhs->var_reg, TYPE_NUMBER);
        move(lhs->var_reg, dest);
        gen_code("sarq $1, %%%s", dest);
        gen_code("imulq %%%s, %%%s", rhs->var_reg, dest);
    } else {
        /* t_expr * t_expr, type checked earlier */
        move(lhs->reg, dest);
        gen_code("sarq $1, %%%s", dest);
        gen_code("imulq %%%s, %%%s", rhs->reg, dest);
    }
}

/**
 *  result is tagged because var is tagged
 */
void gen_mul_var_const(struct tree *node)
{
    struct tree *lhs = LEFT_CHILD(node);
    struct tree *rhs = RIGHT_CHILD(node);

    const char *dest = node->reg;

    struct tree *constnode = lhs->constant ? lhs : rhs;
    struct tree *varnode = lhs->op == OP_VAR ? lhs : rhs;

    expect(varnode->var_reg, TYPE_NUMBER);
    gen_mul_reg_const(varnode->var_reg, dest, constnode->value);
}

static void gen_eq_const(long int value, const char *source, const char *dest)
{
    value = tag_const(value);

    gen_code("xorq %%%s, %%%s", dest);
    gen_code("testq $%ld, %%%s", value, source);
    gen_code("cmovqe $2, %%%s", dest);
}

void gen_eq(struct tree *node)
{
    struct tree *lhs = LEFT_CHILD(node);
    struct tree *rhs = RIGHT_CHILD(node);

    const char *dest = node->reg;

    if(lhs->constant && rhs->constant) {
        gen_code("TODO: upps, constant fold!");
    } else if(lhs->constant) {
        if(rhs->op == OP_VAR)
            gen_eq_const(lhs->value, rhs->var_reg, dest);
        else
            gen_eq_const(lhs->value, rhs->reg, dest);
    } else if(rhs->constant) {
        if(lhs->op == OP_VAR)
            gen_eq_const(rhs->value, lhs->var_reg, dest);
        else
            gen_eq_const(rhs->value, lhs->reg, dest);
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

void gen_islist(struct tree *node)
{
    struct tree *lhs = LEFT_CHILD(node);

    const char *source;
    const char *dest = node->reg;

    if(lhs->op == OP_VAR) {
        source = lhs->var_reg;
    } else {
        source = lhs->reg;
    }
    move(source, dest);
    gen_code("andq $3, %%%s", dest);
    gen_code("testq $2, %%%s", dest);
    gen_code("jz .nolist");
    gen_code("movq $2 %%%s", dest);
    gen_code("jmp .after");
    gen_code(".nolist:");
    gen_code("movq $0, %%%s", dest);
    gen_code(".after:");
}

