#ifndef TREE_H
#define TREE_H

#include "stdio.h"


/* iburg macros */
#define LEFT_CHILD(t) ((t)->kids[0])
#define RIGHT_CHILD(t) ((t)->kids[1])
#define STATE_LABEL(t) ((t)->state)
#define OP_LABEL(t) ((t)->op)
#define PANIC printf
#define NODEPTR_TYPE struct tree*
#define STATE_TYPE struct burm_state*

enum {
    OP_VAR = 1,
    OP_NUM,
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_AND,
    OP_LT,
    OP_EQ,
    OP_NOT,
    OP_HEAD,
    OP_TAIL,
    OP_ISNUM,
    OP_ISLIST,
    OP_ISFUN,
    OP_RET,
    OP_UNKNOWN
};

struct tree {
    int op;                 /* node type */
    struct tree *kids[2];   /* child nodes */

    char *reg;
    char *var_reg;          /* hold register where variable is saved, if OP_VAR */
    char *name;
    long int value;             /* expression results */


    struct burm_state *state; /* BURG state variable */
};


struct tree *new_node(int op, struct tree *left, struct tree *right);
struct tree *new_node_with_reg(int op, struct tree *left, struct tree *right, char *reg);
struct tree *new_const_node(int op, struct tree *left, struct tree *right, long int value);
struct tree *new_ident_node(int op, struct tree *left, struct tree *right, const char *name, const char *var_reg);

void print_tree(struct tree *node, int indent);

#endif // TREE_H
