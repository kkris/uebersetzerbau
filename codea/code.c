#include "stdio.h"
#include "assert.h"
#include "node.h"
#include "codegen.h"
#include <limits.h>
#include <stdlib.h>
#ifndef STATE_TYPE
#define STATE_TYPE int
#endif
#ifndef ALLOC
#define ALLOC(n) malloc(n)
#endif
#ifndef burm_assert
#define burm_assert(x,y) if (!(x)) { y; abort(); }
#endif

#define burm_reg_NT 1
#define burm_num_NT 2
int burm_max_nt = 2;

struct burm_state {
	int op;
	struct burm_state *left, *right;
	short cost[3];
	struct {
		unsigned burm_reg:3;
		unsigned burm_num:2;
	} rule;
};

static short burm_nts_0[] = { burm_reg_NT, burm_reg_NT, 0 };
static short burm_nts_1[] = { burm_reg_NT, burm_num_NT, 0 };
static short burm_nts_2[] = { burm_num_NT, burm_num_NT, 0 };
static short burm_nts_3[] = { 0 };

short *burm_nts[] = {
	0,	/* 0 */
	burm_nts_0,	/* 1 */
	burm_nts_1,	/* 2 */
	burm_nts_0,	/* 3 */
	burm_nts_1,	/* 4 */
	burm_nts_2,	/* 5 */
	burm_nts_3,	/* 6 */
	burm_nts_3,	/* 7 */
};

char burm_arity[] = {
	0,	/* 0 */
	2,	/* 1=ADD */
	0,	/* 2=REG */
	0,	/* 3=NUM */
	2,	/* 4=ASSIGN */
	2,	/* 5=ADDASSIGN */
};

static short burm_decode_reg[] = {
	0,
	1,
	2,
	3,
	4,
	6,
};

static short burm_decode_num[] = {
	0,
	5,
	7,
};

int burm_rule(STATE_TYPE state, int goalnt) {
	burm_assert(goalnt >= 1 && goalnt <= 2, PANIC("Bad goal nonterminal %d in burm_rule\n", goalnt));
	if (!state)
		return 0;
	switch (goalnt) {
	case burm_reg_NT:	return burm_decode_reg[((struct burm_state *)state)->rule.burm_reg];
	case burm_num_NT:	return burm_decode_num[((struct burm_state *)state)->rule.burm_num];
	default:
		burm_assert(0, PANIC("Bad goal nonterminal %d in burm_rule\n", goalnt));
	}
	return 0;
}


STATE_TYPE burm_state(int op, STATE_TYPE left, STATE_TYPE right) {
	int c;
	struct burm_state *p, *l = (struct burm_state *)left,
		*r = (struct burm_state *)right;

	assert(sizeof (STATE_TYPE) >= sizeof (void *));
	if (burm_arity[op] > 0) {
		p = ALLOC(sizeof *p);
		burm_assert(p, PANIC("ALLOC returned NULL in burm_state\n"));
		p->op = op;
		p->left = l;
		p->right = r;
		p->rule.burm_reg = 0;
		p->cost[1] =
		p->cost[2] =
			32767;
	}
	switch (op) {
	case 1: /* ADD */
		assert(l && r);
		{	/* num: ADD(num,num) */
			c = l->cost[burm_num_NT] + r->cost[burm_num_NT] + 0;
			if (c + 0 < p->cost[burm_num_NT]) {
				p->cost[burm_num_NT] = c + 0;
				p->rule.burm_num = 1;
			}
		}
		break;
	case 2: /* REG */
		{
			static struct burm_state z = { 2, 0, 0,
				{	0,
					0,	/* reg: REG */
					32767,
				},{
					5,	/* reg: REG */
					0,
				}
			};
			return (STATE_TYPE)&z;
		}
	case 3: /* NUM */
		{
			static struct burm_state z = { 3, 0, 0,
				{	0,
					32767,
					0,	/* num: NUM */
				},{
					0,
					2,	/* num: NUM */
				}
			};
			return (STATE_TYPE)&z;
		}
	case 4: /* ASSIGN */
		assert(l && r);
		{	/* reg: ASSIGN(reg,num) */
			c = l->cost[burm_reg_NT] + r->cost[burm_num_NT] + 1;
			if (c + 0 < p->cost[burm_reg_NT]) {
				p->cost[burm_reg_NT] = c + 0;
				p->rule.burm_reg = 2;
			}
		}
		{	/* reg: ASSIGN(reg,reg) */
			c = l->cost[burm_reg_NT] + r->cost[burm_reg_NT] + 1;
			if (c + 0 < p->cost[burm_reg_NT]) {
				p->cost[burm_reg_NT] = c + 0;
				p->rule.burm_reg = 1;
			}
		}
		break;
	case 5: /* ADDASSIGN */
		assert(l && r);
		{	/* reg: ADDASSIGN(reg,num) */
			c = l->cost[burm_reg_NT] + r->cost[burm_num_NT] + 1;
			if (c + 0 < p->cost[burm_reg_NT]) {
				p->cost[burm_reg_NT] = c + 0;
				p->rule.burm_reg = 4;
			}
		}
		{	/* reg: ADDASSIGN(reg,reg) */
			c = l->cost[burm_reg_NT] + r->cost[burm_reg_NT] + 1;
			if (c + 0 < p->cost[burm_reg_NT]) {
				p->cost[burm_reg_NT] = c + 0;
				p->rule.burm_reg = 3;
			}
		}
		break;
	default:
		burm_assert(0, PANIC("Bad operator %d in burm_state\n", op));
	}
	return (STATE_TYPE)p;
}

#ifdef STATE_LABEL
static void burm_label1(NODEPTR_TYPE p) {
	burm_assert(p, PANIC("NULL tree in burm_label\n"));
	switch (burm_arity[OP_LABEL(p)]) {
	case 0:
		STATE_LABEL(p) = burm_state(OP_LABEL(p), 0, 0);
		break;
	case 1:
		burm_label1(LEFT_CHILD(p));
		STATE_LABEL(p) = burm_state(OP_LABEL(p),
			STATE_LABEL(LEFT_CHILD(p)), 0);
		break;
	case 2:
		burm_label1(LEFT_CHILD(p));
		burm_label1(RIGHT_CHILD(p));
		STATE_LABEL(p) = burm_state(OP_LABEL(p),
			STATE_LABEL(LEFT_CHILD(p)),
			STATE_LABEL(RIGHT_CHILD(p)));
		break;
	}
}

STATE_TYPE burm_label(NODEPTR_TYPE p) {
	burm_label1(p);
	return ((struct burm_state *)STATE_LABEL(p))->rule.burm_reg ? STATE_LABEL(p) : 0;
}

NODEPTR_TYPE *burm_kids(NODEPTR_TYPE p, int eruleno, NODEPTR_TYPE kids[]) {
	burm_assert(p, PANIC("NULL tree in burm_kids\n"));
	burm_assert(kids, PANIC("NULL kids in burm_kids\n"));
	switch (eruleno) {
	case 5: /* num: ADD(num,num) */
	case 4: /* reg: ADDASSIGN(reg,num) */
	case 3: /* reg: ADDASSIGN(reg,reg) */
	case 2: /* reg: ASSIGN(reg,num) */
	case 1: /* reg: ASSIGN(reg,reg) */
		kids[0] = LEFT_CHILD(p);
		kids[1] = RIGHT_CHILD(p);
		break;
	case 7: /* num: NUM */
	case 6: /* reg: REG */
		break;
	default:
		burm_assert(0, PANIC("Bad external rule number %d in burm_kids\n", eruleno));
	}
	return kids;
}

int burm_op_label(NODEPTR_TYPE p) {
	burm_assert(p, PANIC("NULL tree in burm_op_label\n"));
	return OP_LABEL(p);
}

STATE_TYPE burm_state_label(NODEPTR_TYPE p) {
	burm_assert(p, PANIC("NULL tree in burm_state_label\n"));
	return STATE_LABEL(p);
}

NODEPTR_TYPE burm_child(NODEPTR_TYPE p, int index) {
	burm_assert(p, PANIC("NULL tree in burm_child\n"));
	switch (index) {
	case 0:	return LEFT_CHILD(p);
	case 1:	return RIGHT_CHILD(p);
	}
	burm_assert(0, PANIC("Bad index %d in burm_child\n", index));
	return 0;
}

#endif
extern node *root;
extern int yyparse(void);
void burm_reduce(NODEPTR_TYPE bnode, int goalnt);
void invoke_burm(NODEPTR_TYPE root) {
burm_label(root);
burm_reduce(root, 1);
}
void burm_reduce(NODEPTR_TYPE bnode, int goalnt)
{
  int ruleNo = burm_rule (STATE_LABEL(bnode), goalnt);
  short *nts = burm_nts[ruleNo];
  NODEPTR_TYPE kids[100];
  int i;

  if (ruleNo==0) {
    fprintf(stderr, "tree cannot be derived from start symbol");
    exit(1);
  }
  burm_kids (bnode, ruleNo, kids);
  for (i = 0; nts[i]; i++)
    burm_reduce (kids[i], nts[i]);    /* reduce kids */

#if DEBUG
  printf ("%s", burm_string[ruleNo]);  /* display rule */
#endif

  switch (ruleNo) {
  case 1:
 gen_code("movq");
    break;
  case 2:
 gen_code("movq");
    break;
  case 3:
 gen_code("addq");
    break;
  case 4:
 gen_code("addq");
    break;
  case 5:
 bnode->value = kids[0]->value + kids[1]->value;
    break;
  case 6:

    break;
  case 7:

    break;
  default:    assert (0);
  }
}
