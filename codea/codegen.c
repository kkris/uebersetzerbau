#include "codegen.h"

#include <stdio.h>
#include <stdarg.h>

static int reg_idx = 0;

char *get_reg() {
    char* registers[] = {"rsi", "rdx", "rcx", "r8", "r9", "r10", "r11"};
    char *reg = registers[reg_idx];
    reg_idx++;

    return reg;
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

void move(long int value, const char *reg)
{
    printf("movq %ld, %s\n", value, reg);
}

void tag(int type, const char *reg)
{
    if(type == TYPE_NUMBER) {
        gen_code("sal %%%s, %%%s", reg, reg);
    } else {
        printf("Not implemented\n");
    }
}

void untag(int type, const char *reg)
{
    if(type == TYPE_NUMBER) {
        gen_code("sar %%%s, %%%s", reg, reg);
    } else {
        printf("Not implemented");
    }
}

void ret(int type, struct tree *node)
{
    if(type == TYPE_NUMBER) {
        gen_code("movq $%ld, %rax", node->value);
        tag(TYPE_NUMBER, "rax");
    } else if(type == TYPE_REG) {
        gen_code("movq %%%s, %%%s", node->reg, "rax");
    }

    gen_code("ret");
}

void gen_not(const char *source, const char *dest)
{
    gen_code("movq $1, %%%s", dest);
    gen_code("xorq %%%s, %%%s", source, dest);
}

