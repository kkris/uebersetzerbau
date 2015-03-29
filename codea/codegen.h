#ifndef CODEGEN_H
#define CODEGEN_H

#include <stdio.h>
#include <stdarg.h>

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

void ret()
{
    gen_code("ret");
}

#endif // CODEGEN_H
