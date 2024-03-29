#include "tree.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct tree *new_node(int op, struct tree *left, struct tree *right)
{
    struct tree *t = malloc(sizeof(struct tree));

    t->op = op;
    LEFT_CHILD(t) = left;
    RIGHT_CHILD(t) = right;
    t->reg = NULL;
    t->name = NULL;
    t->value = 0;
    t->constant = 0;
    t->atomic = 0;
    t->symbol = NULL;

    return t;
}

struct tree *new_const_node(int op, struct tree *left, struct tree *right, long int value)
{
    struct tree *t = new_node(op, left, right);

    make_constant(t, value);

    return t;
}

struct tree *new_ident_node(int op, struct tree *left, struct tree *right, const char *name, struct symbol *symbol)
{
    struct tree *t = new_node(op, left, right);

    t->name = strdup(name);
    t->atomic = 1;

    if(symbol->type == SYMBOL_TYPE_FUN)
        t->op = OP_FUN;

    t->symbol = symbol;

    return t;
}

void make_constant(struct tree *node, long int value)
{
    node->value = value;
    node->constant = 1;
    node->atomic = 1;

    if(value == 0)
        node->op = OP_ZERO;
    else if(value == 1)
        node->op = OP_ONE;
    else
        node->op = OP_NUM;
}

void make_equal_to(struct tree *dest, struct tree *source)
{
    dest->op = source->op;
    dest->reg = source->reg == NULL ? NULL : strdup(source->reg);
    dest->name = source->name == NULL ? NULL : strdup(source->name);
    dest->value = source->value;
    dest->constant = source->constant;
    dest->atomic = source->atomic;
    dest->symbol = source->symbol;

    LEFT_CHILD(dest) = LEFT_CHILD(source);
    RIGHT_CHILD(dest) = RIGHT_CHILD(source);
}

int is_const_or_atomic(struct tree *node)
{
    return node->constant || node->atomic;
}

