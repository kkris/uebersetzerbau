#include "tree.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *opnames[] = {
    "var",
    "num",
    "add",
    "sub",
    "mul",
    "and",
    "lt",
    "eq",
    "not",
    "head",
    "tail",
    "isnum",
    "islist",
    "isfun",
    "ret",
    "unknown"
};


struct tree *new_node(int op, struct tree *left, struct tree *right)
{
    struct tree *t = malloc(sizeof(struct tree));

    t->op = op;
    LEFT_CHILD(t) = left;
    RIGHT_CHILD(t) = right;
    t->reg = NULL;
    t->var_reg = NULL;
    t->name = NULL;
    t->value = 0;

    return t;
}

struct tree *new_node_with_reg(int op, struct tree *left, struct tree *right, char *reg)
{
    struct tree *t = new_node(op, left, right);

    t->reg = strdup(reg);

    return t;
}

struct tree *new_const_node(int op, struct tree *left, struct tree *right, long int value)
{
    struct tree *t = new_node(op, left, right);

    t->value = value;

    return t;
}

struct tree *new_ident_node(int op, struct tree *left, struct tree *right, const char *name, const char *var_reg)
{
    fprintf(stderr, "new named node: (%s, %%%s)\n", name, var_reg);
    struct tree *t = new_node(op, left, right);

    t->name = strdup(name);
    t->var_reg = strdup(var_reg);

    return t;
}



void print_indent(int indent) {
	int i;
	for(i = 0; i < indent; i++) {
		fprintf(stderr, "|");
	}
}

void print_tree(struct tree *node, int indent) {
	print_indent(indent);
	fprintf(stderr, "%s, %s, %s\n", opnames[node->op - 1], node->name, node->reg);
	if(node->kids[0] != NULL || node->kids[1] != NULL) {
		if(node->kids[0] != NULL) {
			print_tree(node->kids[0], indent+1);
		}
		if(node->kids[1] != NULL) {
			print_tree(node->kids[1], indent+1);
		}
	}
}


