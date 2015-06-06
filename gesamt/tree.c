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
    t->data = NULL;

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

    if(symbol != NULL && symbol->type == SYMBOL_TYPE_FUN)
        t->op = OP_FUN;

    t->symbol = symbol;

    return t;
}

struct tree *new_if_node(struct tree *pred, struct tree *then, struct tree *otherwise, int labelno)
{
    struct label_pair *labels = malloc(sizeof(struct label_pair));
    labels->else_label = malloc(10 * sizeof(char*));
    sprintf(labels->else_label, "else%d", labelno);

    labels->epilog_label = malloc(10 * sizeof(char*));
    sprintf(labels->epilog_label, "epilog%d", labelno);

    struct tree *ifnode = new_node(OP_IF, pred, NULL);
    struct tree *ifthen = new_node(OP_IF_THEN, ifnode, then);
    struct tree *t = new_node(OP_IF_THEN_ELSE, ifthen, otherwise);

    ifnode->data = (void*)labels;
    ifthen->data = (void*)labels;
    t->data = (void*)labels;

    return t;
}

struct tree *new_let_node(struct tree *var, struct tree *expr, struct symbol *symbol)
{
    struct tree *assign = new_node(OP_LET_ASSIGN, var, NULL);
    struct tree *let = new_node(OP_LET_EVAL, assign, expr);

    assign->symbol = symbol;
    let->symbol = symbol;

    return let;
}

struct tree *new_lambda_node(struct tree *body, int lambdano)
{
    struct closure_data *data = malloc(sizeof(struct closure_data));
    data->num = lambdano;

    struct tree *prolog = new_node(OP_LAMBDA_PROLOG, NULL, NULL);
    struct tree *t = new_node(OP_LAMBDA, prolog, body);

    prolog->data = (void*)data;
    t->data = (void*)data;

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
    dest->data = source->data;

    LEFT_CHILD(dest) = LEFT_CHILD(source);
    RIGHT_CHILD(dest) = RIGHT_CHILD(source);
}

int is_const_or_atomic(struct tree *node)
{
    return node->constant || node->atomic;
}

