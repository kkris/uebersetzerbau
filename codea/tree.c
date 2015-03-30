#include "tree.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


struct tree *new_node(int op, struct tree *left, struct tree *right) {
    struct tree *t = malloc(sizeof(struct tree));

    t->op = op;
    LEFT_CHILD(t) = left;
    RIGHT_CHILD(t) = right;
    t->reg = NULL;
    t->name = NULL;
    t->value = 0;

    return t;
}

struct tree *new_const_node(int op, struct tree *left, struct tree *right, long int value) {
    struct tree *t = new_node(op, left, right);

    t->value = value;

    return t;
}

struct tree *new_named_node(int op, struct tree *left, struct tree *right, const char *name, const char *reg) {
    fprintf(stderr, "new named node: (%s, %%%s)\n", name, reg);
    struct tree *t = new_node(op, left, right);

    t->name = strdup(name);
    t->reg = strdup(reg);

    return t;
}
