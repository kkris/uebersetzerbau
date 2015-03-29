#ifndef NODE_H
#define NODE_H

#include "stdio.h"


/* iburg macros */
#define LEFT_CHILD(n) ((n)->kids[0])
#define RIGHT_CHILD(n) ((n)->kids[1])
#define STATE_LABEL(n) ((n)->state)
#define OP_LABEL(n) ((n)->op)
#define PANIC printf
#define NODEPTR_TYPE node*

enum {
    OP_UNKNOWN = 1,
    OP_NUM
};

typedef struct node {
    int op;                 /* node type */
    struct node *kids[2];   /* child nodes */

    long value;             /* for constant number nodes */

    struct burm_state *state; /* BURG state variable */
} node;

node *node_new(int op, node *left, node *right);
node *constant_number_node_new(int op, node *left, node *right, long value);

#endif // NODE_H
