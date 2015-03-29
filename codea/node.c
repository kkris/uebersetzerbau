#include "node.h"

#include <stdlib.h>


node *node_new(int op, node *left, node *right) {
    node *n = malloc(sizeof(node));

    n->op = op;
    LEFT_CHILD(n) = left;
    RIGHT_CHILD(n) = right;
    n->value = 0;

    return n;
}

node *constant_number_node_new(int op, node *left, node *right, long value) {
    node *n = node_new(op, left, right);

    n->value = value;

    return n;
}
