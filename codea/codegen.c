#include "codegen.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>


char *get_next_reg(const char *prev) {
    fprintf(stderr, "next_reg(%s, ", prev);
    if(prev == NULL)
        return strdup("rdi");

    char* registers[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9", "r10"};

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

void tag(int type, const char *reg)
{
    if(type == TYPE_NUMBER) {
        gen_code("sal %%%s, %%%s", reg, reg);
    } else {
        printf("Not implemented\n");
    }
}

void untag(const char *reg)
{
    // TODO: other types
    gen_code("sar %%%s, %%%s", reg, reg);
}

void load_num(const char *var_reg, const char *dest)
{
    gen_code("bt %%%s", var_reg);
    gen_code("jc .raisesig");
    gen_code("sar %%%s, %%%s", var_reg, dest);
}

void ret(struct tree *node, int tag_type, int type)
{
    if(tag_type == TAGGED) {
        move(node->reg, "rax");
    } else {
        if(type == TYPE_NUMBER) {
            long int tagged = node->value << 1;
            gen_code("moveq $%ld, %rax", tagged);
        }

    }

    gen_code("ret");
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

