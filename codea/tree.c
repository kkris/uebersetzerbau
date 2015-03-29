#include "tree.h"

#include <stdio.h>
#include <stdlib.h>


struct tree *new_node(int op, struct tree *left, struct tree *right) {
    struct tree *t = malloc(sizeof(struct tree));

    t->op = op;
    LEFT_CHILD(t) = left;
    RIGHT_CHILD(t) = right;
    t->value = 0;

    return t;
}

struct tree *new_const_node(int op, struct tree *left, struct tree *right, long int value) {
    struct tree *t = new_node(op, left, right);

    t->value = value;

    return t;
}
