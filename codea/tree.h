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
    OP_UNKNOWN = 1,
    OP_NUM,
    OP_ADD
};

struct tree {
    int op;                 /* node type */
    struct tree *kids[2];   /* child nodes */

    long int value;             /* expression results */

    struct burm_state *state; /* BURG state variable */
};


struct tree *new_node(int op, struct tree *left, struct tree *right);
struct tree *new_const_node(int op, struct tree *left, struct tree *right, long int value);

#endif // TREE_H
