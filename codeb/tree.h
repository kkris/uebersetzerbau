#ifndef TREE_H
#define TREE_H

#include "stdio.h"

#include "symbol.h"


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
    OP_ZERO,
    OP_ONE,
    OP_LIST,
    OP_FUN,
    OP_IF,
    OP_THEN_ELSE,
    OP_UNKNOWN
};

struct tree {
    int op;                 /* node type */
    struct tree *kids[2];   /* child nodes */

    char *reg;
    char *name;
    long int value;             /* expression results */

    int constant;
    int atomic;                 /* 1 if node is ident or constant term */

    struct burm_state *state; /* BURG state variable */

    struct symbol *symbol; /* corresponding symbol if available */
};


struct tree *new_node(int op, struct tree *left, struct tree *right);
struct tree *new_const_node(int op, struct tree *left, struct tree *right, long int value);
struct tree *new_ident_node(int op, struct tree *left, struct tree *right, const char *name, struct symbol *symbol);
struct tree *new_if_node(struct tree *pred, struct tree *then, struct tree *otherwise);

void make_constant(struct tree *node, long int value);
void make_equal_to(struct tree *dest, struct tree *source);

int is_const_or_atomic(struct tree *node);

#endif // TREE_H
