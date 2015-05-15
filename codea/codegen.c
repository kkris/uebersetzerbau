#include "codegen.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>

const char *heap_ptr = "r10";
const char *temp_reg = "r11";

int heap_ptr_initialized = 0;

// global variable which tracks the just tagged register
char *just_tagged = NULL;
char *just_untagged = NULL;

static char *type_checked_vars[10] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};

static void reset_state()
{
    heap_ptr_initialized = 0;
    just_tagged = NULL;
    just_untagged = NULL;

    int i;
    for(i = 0; i < 10; i++) {
        type_checked_vars[i] = NULL;
    }
}

void maybe_force_tag_or_untag();

void debug(const char *msg, ...)
{
    return;

    va_list ap;

    va_start(ap, msg);
    vfprintf(stderr, msg, ap);
    fprintf(stderr, "\n");
    va_end(ap);
}

char *get_next_reg(const char *prev, int reuse)
{
    fprintf(stderr, "next_reg(%s, ", prev);
    if(prev == NULL)
        return strdup("rdi");

    char* registers[] = {"rdi", "rax", "rsi", "rdx", "rcx", "r8", "r9"};

    int i = 0;
    for(; i < sizeof(registers) / sizeof(registers[0]) - 1; i++) {
        if(strcmp(prev, registers[i]) == 0) {
            fprintf(stderr, "%s)\n", registers[i + 1]);
            if(reuse == 1)
                return strdup(registers[i]);
            else
                return strdup(registers[i + 1]);
        }
    }

    debug("Upps, no register left. Implement a better allocation algorithm!");

    return NULL;
}



void gen_code(const char *code, ...)
{
    maybe_force_tag_or_untag();

    va_list ap;

    va_start(ap, code);
    fprintf(stdout, "\t");
    vfprintf(stdout, code, ap);
    fprintf(stdout, "\n");
    va_end(ap);
}

void gen_func(const char *name)
{
    printf(".globl %s\n%s:\n", name, name);
}

void move(const char *source, const char *dest)
{
    if(strcmp(source, dest) == 0)
        return; // nop

    gen_code("movq %%%s, %%%s", source, dest);
}

static void move_const(long int value, const char *dest)
{
    gen_code("movq $%ld, %%%s", value, dest);
}

void lea(long int offset, const char *base, const char *index, int scale, const char *dest)
{
    if(offset == 0) {
        if(base == NULL) {
            if(scale == 1) {
                move(index, dest);
            } else {
                gen_code("leaq (, %%%s, %d), %%%s", index, scale, dest);
            }
        } else {
            if(index == NULL) {
                move(base, dest);
            } else {
                if(scale == 1) {
                    gen_code("leaq (%%%s, %%%s), %%%s", base, index, dest);
                } else {
                    gen_code("leaq (%%%s, %%%s, %d), %%%s", base, index, scale, dest);
                }
            }
        }
    } else {
        if(base == NULL) {
            if(scale == 1) {
                gen_code("leaq %ld(, %%%s), %%%s", offset, index, dest);
            } else {
                gen_code("leaq %ld(, %%%s, %d), %%%s", offset, index, scale, dest);
            }
        } else {
            if(index == NULL) {
                gen_code("leaq %ld(%%%s), %%%s", offset, base, dest);
            } else {
                if(scale == 1) {
                    gen_code("leaq %ld(%%%s, %%%s), %%%s", offset, base, index, dest);
                } else {
                    gen_code("leaq %ld(%%%s, %%%s, %d), %%%s", offset, base, index, scale, dest);
                }
            }
        }
    }
}

void lea_offset_base(long int offset, const char *base, const char *dest)
{
    lea(offset, base, NULL, -1, dest);
}

void lea_base_index(const char *base, const char *index, const char *dest)
{
    lea(0, base, index, 1, dest);
}

void raisesig()
{
    gen_code("jmp raisesig");
}

void expect(const char *var_reg, int type)
{
    debug("expect");

    int i;
    for(i = 0; i < 10; i++) {
        if(type_checked_vars[i] == NULL) break;
        if(strcmp(var_reg, type_checked_vars[i]) == 0)
            return;
    }

    if(type == TYPE_NUMBER) {
        gen_code("testq $1, %%%s", var_reg);
        gen_code("jnz raisesig");

        type_checked_vars[i] = strdup(var_reg);
    }
}

void expect_num(struct tree *node)
{
    debug("expect_num");

    if(node->op == OP_VAR)
        expect(node->reg, TYPE_NUMBER);
}

void expect_list(struct tree *node)
{
    debug("expect_list");

    gen_code("testq $1, %%%s", node->reg);
    gen_code("jz raisesig");
    gen_code("testq $2, %%%s", node->reg);
    gen_code("jnz raisesig");
}

void tag(int type, const char *source, const char *dest)
{
    debug("tag(%s, %s)", source, dest);

    if(type == TYPE_NUMBER) {
        move(source, dest);
        gen_code("salq $1, %%%s", dest);
    } else if(type == TYPE_LIST) {
        move(source, dest);
        gen_code("addq $1, %%%s", dest);
    } else {
        printf("Not implemented\n");
    }
}

/**
 * We record the most recently tagged and untagged register and if the
 * complementary operation is seen right after then we emit no code as the
 * combination is a nop.
 */
void maybe_force_tag_or_untag()
{
    if(just_untagged != NULL)
        fprintf(stdout, "\tsarq $1, %%%s\n", just_untagged);
    if(just_tagged != NULL)
        fprintf(stdout, "\tsalq $1, %%%s\n", just_tagged);

    just_untagged = just_tagged = NULL;
}

void tag_num_inplace(const char *reg)
{
    debug("tag_num_inplace(%s)", reg);

    if(just_untagged != NULL && strcmp(just_untagged, reg) == 0) {
        // we just untagged this register. don't untag it in the first place
        just_tagged = NULL;
        just_untagged = NULL;
        return;
    }

    just_tagged = strdup(reg);
}

void untag_num_inplace(const char *reg)
{
    debug("tag_num_inplace(%s)", reg);

    if(just_tagged != NULL && strcmp(just_tagged, reg) == 0) {
        // we just tagged this register. don't tag it in the first place
        just_tagged = NULL;
        just_untagged = NULL;
        return;
    }

    just_untagged = strdup(reg);
}

void tag_list_inplace(const char *reg)
{
    debug("tag_list_inplace(%s)", reg);

    gen_code("addq $1, %%%s", reg);
}

void untag_list_inplace(const char *reg)
{
    debug("untag_list_inplace(%s)", reg);

    gen_code("subq $1, %%%s", reg);
}


long int tag_const(long int value)
{
    return value << 1;
}

void ret(struct tree *node, int tag_type, int type)
{
    if(node->op == OP_VAR) {
        move(node->reg, "rax");
    } else if(tag_type == TAGGED) {
        move(node->reg, "rax");
    } else {
        if(type == TYPE_NUMBER) {
            long int tagged = node->value << 1;
            move_const(tagged, "rax");
        } else if(type == TYPE_REG) {
            move(node->reg, "rax");
            tag_num_inplace("rax");
        } else if(type == TYPE_LIST) {
            move(node->reg, "rax");
            tag(TYPE_LIST, "rax", "rax");
        } else {
            gen_code("ret todo");
        }
    }

    gen_code("ret");

    reset_state();
}

void gen_not(struct tree *node)
{
    debug("gen_not");

    struct tree *lhs = LEFT_CHILD(node);

    const char *dest = node->reg;

    if(lhs->op == OP_VAR) {
        expect_num(lhs);
        move(lhs->reg, dest);
        gen_code("xorq $2, %%%s", dest);
    } else {
        move(lhs->reg, dest);
        gen_code("xorq $2, %%%s", dest);
    }
}

static void gen_and_reg_const(long int value, const char *source, const char *dest)
{
    debug("gen_and_reg_const");

    move(source, dest);
    gen_code("andq $%ld, %%%s", tag_const(value), dest);
}

static void gen_and_reg_reg(const char *s1, const char *s2, const char *dest)
{
    debug("gen_and_reg_reg");

    move(s1, dest);
    gen_code("andq %%%s, %%%s", s2, dest);
}

void gen_and(struct tree *node)
{
    debug("gen_and");

    struct tree *lhs = LEFT_CHILD(node);
    struct tree *rhs = RIGHT_CHILD(node);

    const char *dest = node->reg;

    if(lhs->op == OP_VAR && rhs->op == OP_VAR) {
        if(strcmp(lhs->reg, rhs->reg) == 0)
            move(lhs->reg, dest);
        else
            gen_and_reg_reg(lhs->reg, rhs->reg, dest);
    } else if(lhs->constant) {
        gen_and_reg_const(lhs->value, rhs->reg, dest);
    } else if(rhs->constant) {
        gen_and_reg_const(rhs->value, lhs->reg, dest);
    } else {
        gen_and_reg_reg(lhs->reg, rhs->reg, dest);
    }
}


static void gen_add_reg_const(long int value, const char *source, const char *dest)
{
    debug("gen_add_reg_const");

    value = tag_const(value);

    move(source, dest);
    gen_code("addq $%ld, %%%s", value, dest);
}

void gen_add(struct tree *node)
{
    debug("gen_add");

    struct tree *lhs = LEFT_CHILD(node);
    struct tree *rhs = RIGHT_CHILD(node);

    const char *dest = node->reg;

    if(lhs->constant) {
        lea_offset_base(tag_const(lhs->value), rhs->reg, dest);
    } else if(rhs->constant) {
        lea_offset_base(tag_const(rhs->value), lhs->reg, dest);
    } else {
        lea_base_index(lhs->reg, rhs->reg, dest);
    }
}

static void gen_sub_const_reg(long int value, const char *source, const char *dest)
{
    debug("gen_sub_reg_const");

    move_const(value, dest);
    gen_code("subq %%%s, %%%s", source, dest);
}

static void gen_sub_reg_reg(const char *s1, const char *s2, const char *dest)
{
    debug("gen_sub_reg_reg");

    move(s1, dest);
    gen_code("subq %%%s, %%%s", s2, dest);
}

void gen_sub(struct tree *node)
{
    debug("gen_sub");

    struct tree *lhs = LEFT_CHILD(node);
    struct tree *rhs = RIGHT_CHILD(node);

    const char *dest = node->reg;

    if(lhs->op == OP_VAR && rhs->op == OP_VAR) {
        if(strcmp(lhs->name, rhs->name) == 0)
            move_const(0, dest);
        else
            gen_sub_reg_reg(lhs->reg, rhs->reg, dest);
    } else if(lhs->constant) {
        gen_sub_const_reg(tag_const(lhs->value), rhs->reg, dest);
    } else if(rhs->constant) {
        lea_offset_base(-tag_const(rhs->value), lhs->reg, dest);
    } else {
        gen_sub_reg_reg(lhs->reg, rhs->reg, dest);
    }
}

static void gen_mul_const_reg(long int value, const char *source, const char *dest)
{
    debug("gen_mul_reg_const");

    switch(value) {
    case 0:
    case 1:
        gen_code("mul with 0, 1 should be handled else where"); break;
    case 2:
        lea(0, NULL, source, 2, dest); break;
    case 3:
        lea(0, source, source, 2, dest); break;
    case 4:
        lea(0, NULL, source, 4, dest); break;
    case 5:
        lea(0, source, source, 4, dest); break;
    case 8:
        lea(0, NULL, source, 8, dest); break;
    case 9:
        lea(0, source, source, 8, dest); break;
    default:
        move(source, dest);
        gen_code("imulq $%ld, %%%s", value, dest);
    }
}

static void gen_mul_reg_reg(const char *s1, const char *s2, const char *dest)
{
    debug("gen_mul_reg_reg");

    move(s1, dest);
    gen_code("imulq %%%s, %%%s", s2, dest);
}


void gen_mul(struct tree *node)
{
    debug("gen_mul");

    struct tree *lhs = LEFT_CHILD(node);
    struct tree *rhs = RIGHT_CHILD(node);

    const char *dest = node->reg;

    if(lhs->op == OP_VAR && rhs->op == OP_VAR) {
        move(lhs->reg, dest);
        untag_num_inplace(dest);
        gen_code("imulq %%%s, %%%s", rhs->reg, dest);
        untag_num_inplace(dest);
    } else if(lhs->constant) {
        gen_mul_const_reg(lhs->value, rhs->reg, dest);
    } else if(rhs->constant) {
        gen_mul_const_reg(rhs->value, lhs->reg, dest);
    } else {
        gen_mul_reg_reg(lhs->reg, rhs->reg, dest);
    }

    if(lhs->op == OP_VAR || rhs->op == OP_VAR)
        untag_num_inplace(dest);
}

static void gen_eq_const_reg(long int value, const char *source, const char *dest)
{
    debug("gen_eq_const");

    value = tag_const(value);

    move_const(2, temp_reg);
    gen_code("cmp $%ld, %%%s", value, source);
    gen_code("leaq 0(, 1), %%%s", dest);
    gen_code("cmovz %%%s, %%%s", temp_reg, dest);
}

static void gen_eq_reg_reg(const char *s1, const char *s2, const char *dest)
{
    debug("gen_eq_reg_reg");

    move_const(2, temp_reg);
    gen_code("cmp %%%s, %%%s", s1, s2);
    gen_code("leaq 0(, 1), %%%s", dest);
    gen_code("cmovz %%%s, %%%s", temp_reg, dest);

}

void gen_eq(struct tree *node)
{
    debug("gen_eq");

    struct tree *lhs = LEFT_CHILD(node);
    struct tree *rhs = RIGHT_CHILD(node);

    const char *dest = node->reg;

    if(lhs->op == OP_VAR && rhs->op == OP_VAR) {
        if(strcmp(lhs->reg, rhs->reg) == 0)
            move_const(2, dest);
        else
            gen_eq_reg_reg(lhs->reg, rhs->reg, dest);
    } else if(lhs->constant) {
        gen_eq_const_reg(lhs->value, rhs->reg, dest);
    } else if(rhs->constant) {
        gen_eq_const_reg(rhs->value, lhs->reg, dest);
    } else {
        gen_eq_reg_reg(lhs->reg, rhs->reg, dest);
    }
}

static void gen_lt_reg_reg(const char *s1, const char *s2, const char *dest)
{
    debug("gen_lt_reg_reg");

    move_const(2, temp_reg);
    gen_code("cmp %%%s, %%%s", s1, s2);
    gen_code("leaq 0(, 1), %%%s", dest);
    gen_code("cmovg %%%s, %%%s", temp_reg, dest);
}

static void gen_lt_const_reg(long int value, const char *source, const char *dest)
{
    debug("gen_lt_const");

    move_const(2, temp_reg);
    gen_code("cmp $%ld, %%%s", tag_const(value), source);
    gen_code("leaq 0(, 1), %%%s", dest);
    gen_code("cmovg %%%s, %%%s", temp_reg, dest);
}

static void gen_lt_reg_const(long int value, const char *source, const char *dest)
{
    debug("gen_lt_const");

    move_const(2, temp_reg);
    gen_code("cmp $%ld, %%%s", tag_const(value), source);
    gen_code("leaq 0(, 1), %%%s", dest);
    gen_code("cmovl %%%s, %%%s", temp_reg, dest);
}


void gen_lt(struct tree *node)
{
    debug("gen_lt");

    struct tree *lhs = LEFT_CHILD(node);
    struct tree *rhs = RIGHT_CHILD(node);

    const char *dest = node->reg;

    if(lhs->op == OP_VAR && rhs->op == OP_VAR) {
        if(strcmp(lhs->reg, rhs->reg) == 0)
            move_const(0, dest);
        else
            gen_lt_reg_reg(lhs->reg, rhs->reg, dest);
    } else if(lhs->constant) {
        gen_lt_const_reg(lhs->value, rhs->reg, dest);
    } else if(rhs->constant) {
        gen_lt_reg_const(rhs->value, lhs->reg, dest);
    } else {
        gen_lt_reg_reg(lhs->reg, rhs->reg, dest);
    }
}

void gen_isnum(struct tree *node)
{
    debug("gen_isnum");

    struct tree *lhs = LEFT_CHILD(node);
    const char *dest = node->reg;

    if(lhs->op == OP_VAR) {
        move(lhs->reg, dest);
        gen_code("andq $1, %%%s", dest);
        gen_code("xorq $1, %%%s", dest);
    } else {
        gen_code("UNSUPPORTED");
    }
}

void gen_islist(struct tree *node)
{
    debug("gen_islist");

    struct tree *lhs = LEFT_CHILD(node);

    const char *source;
    const char *dest = node->reg;

    if(lhs->op == OP_VAR) {
        source = lhs->reg;
    } else {
        source = lhs->reg;
    }

    move_const(0, temp_reg);

    move(source, dest);
    gen_code("andq $3, %%%s", dest);
    gen_code("cmpq $1, %%%s", dest);
    gen_code("lea 0(,1), %%%s", dest);
    gen_code("cmovz %%%s, %%%s", temp_reg, dest);
}

void gen_head(struct tree *node)
{
    debug("gen_head");

    struct tree *lhs = LEFT_CHILD(node);

    const char *dest = node->reg;

    gen_code("movq (%%%s), %%%s", lhs->reg, dest);
}

static void set_head_const(const char *list_reg, long int value)
{
    gen_code("movq $%ld, (%%%s)", tag_const(value), list_reg);
}

static void set_head_reg(const char *list_reg, const char *reg)
{
    gen_code("movq %%%s, (%%%s)", reg, list_reg);
}

static void set_tail_const(const char *list_reg, long int value)
{
    gen_code("movq $%ld, 8(%%%s)", tag_const(value), list_reg);
}

static void set_tail_reg(const char *list_reg, const char *reg)
{
    gen_code("movq %%%s, 8(%%%s)", reg, list_reg);
}

void gen_list(struct tree *node)
{
    debug("gen_list");

    struct tree *lhs = LEFT_CHILD(node);
    struct tree *rhs = RIGHT_CHILD(node);

    const char *dest = node->reg;

    if(heap_ptr_initialized == 0) {
        gen_code("movq %%%s, %%%s", "r15", heap_ptr);
        heap_ptr_initialized = 1;
    }

    if(lhs->constant)
        gen_code("movq $%ld, (%%%s)", tag_const(lhs->value), heap_ptr);
    else
        gen_code("movq %%%s, (%%%s)", lhs->reg, heap_ptr);

    if(rhs->constant)
        gen_code("movq $%ld, 8(%%%s)", tag_const(rhs->value), heap_ptr);
    else
        gen_code("movq %%%s, 8(%%%s)", rhs->reg, heap_ptr);


    // return cell address and advance heap ptr
    move(heap_ptr, dest);
    gen_code("addq $16, %%%s", heap_ptr);
}
