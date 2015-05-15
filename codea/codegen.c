#include "codegen.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>

const char *heap_ptr = "r10";
const char *temp_reg = "r11";

void debug(const char *msg, ...)
{
    return;

    va_list ap;

    va_start(ap, msg);
    vfprintf(stderr, msg, ap);
    fprintf(stderr, "\n");
    va_end(ap);
}

static char *type_checked_vars[5] = {NULL, NULL, NULL, NULL, NULL};

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

char *get_lower_byte_reg(const char *reg)
{
    if(strcmp(reg, "rax") == 0) return strdup("al");
    if(strcmp(reg, "rbx") == 0) return strdup("bl");
    if(strcmp(reg, "rcx") == 0) return strdup("cl");
    if(strcmp(reg, "rdx") == 0) return strdup("dl");
    if(strcmp(reg, "rsi") == 0) return strdup("sil");
    if(strcmp(reg, "rdi") == 0) return strdup("dil");
    gen_code("TODO");
}

void gen_code(const char *code, ...)
{
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

void expect(const char *var_reg, int type)
{
    debug("expect");

    int i;
    for(i = 0; i < 5; i++) {
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
        expect(node->var_reg, TYPE_NUMBER);
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

void tag_num_inplace(const char *reg)
{
    debug("tag_num_inplace(%s)", reg);

    gen_code("salq $1, %%%s", reg);
}

void untag_num_inplace(const char *reg)
{
    debug("tag_num_inplace(%s)", reg);

    gen_code("sarq $1, %%%s", reg);
}

long int tag_const(long int value)
{
    return value << 1;
}

void ret(struct tree *node, int tag_type, int type)
{
    if(node->op == OP_VAR) {
        move(node->var_reg, "rax");
    } else if(tag_type == TAGGED) {
        move(node->reg, "rax");
    } else {
        if(type == TYPE_NUMBER) {
            long int tagged = node->value << 1;
            gen_code("movq $%ld, %rax", tagged);
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

    int i;
    for(i = 0; i < 5; i++) {
        type_checked_vars[i] = NULL;
    }
}

void gen_not(struct tree *node)
{
    debug("gen_not");

    struct tree *lhs = LEFT_CHILD(node);

    const char *dest = node->reg;

    if(lhs->op == OP_VAR) {
        expect_num(lhs);
        move(lhs->var_reg, dest);
        gen_code("xorq $2, %%%s", dest);
    } else {
        move(lhs->reg, dest);
        gen_code("xorq $2, %%%s", dest);
    }
}

static void gen_and_reg_const(long int value, const char *source, const char *dest, int tag_type)
{
    debug("gen_and_reg_const");

    if(tag_type == TAGGED)
        value = tag_const(value);

    move(source, dest);
    gen_code("andq $%ld, %%%s", value, dest);
}

void gen_and(struct tree *node, int tag_type)
{
    debug("gen_and");

    struct tree *lhs = LEFT_CHILD(node);
    struct tree *rhs = RIGHT_CHILD(node);

    const char *dest = node->reg;

    if(lhs->constant && rhs->constant) {
        gen_code("TODO: upps, constant fold!");
    } else if(lhs->constant) {
        gen_and_reg_const(lhs->value, rhs->reg, dest, tag_type);
    } else if(rhs->constant) {
        gen_and_reg_const(rhs->value, lhs->reg, dest, tag_type);
    } else if(lhs->op == OP_VAR && rhs->op == OP_VAR){
        expect(lhs->var_reg, TYPE_NUMBER);
        expect(rhs->var_reg, TYPE_NUMBER);
        if(strcmp(lhs->name, rhs->name) == 0)
            move(lhs->var_reg, dest);
        else {
            move(lhs->var_reg, dest);
            gen_code("andq %%%s, %%%s", rhs->var_reg, dest);
        }
    } else {
        if(lhs->op == OP_VAR) {
            move(lhs->var_reg, dest);
            gen_code("andq %%%s, %%%s", rhs->reg, dest);
        } else if(rhs->op == OP_VAR){
            move(lhs->reg, dest);
            gen_code("andq %%%s, %%%s", rhs->var_reg, dest);
        } else {
            move(lhs->reg, dest);
            gen_code("andq %%%s, %%%s", rhs->reg, dest);
        }
    }
}

void gen_and_var_const(struct tree *node, int tag_type)
{
    debug("gen_and_var_const");

    struct tree *lhs = LEFT_CHILD(node);
    struct tree *rhs = RIGHT_CHILD(node);

    const char *dest = node->reg;

    struct tree *constnode = lhs->constant ? lhs : rhs;
    struct tree *varnode = lhs->op == OP_VAR ? lhs : rhs;

    long int value = constnode->value;
    if(tag_type == TAGGED)
        value = tag_const(value);

    expect(varnode->var_reg, TYPE_NUMBER);

    gen_and_reg_const(constnode->value, varnode->var_reg, dest, tag_type);
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
        gen_add_reg_const(lhs->value, rhs->reg, dest);
    } else if(rhs->constant) {
        gen_add_reg_const(rhs->value, lhs->reg, dest);
    } else if(lhs->op == OP_VAR && rhs->op == OP_VAR){
        expect(lhs->var_reg, TYPE_NUMBER);
        expect(rhs->var_reg, TYPE_NUMBER);
        lea_base_index(lhs->var_reg, rhs->var_reg, dest);
    } else {
        if(lhs->op == OP_VAR) {
            lea_base_index(lhs->var_reg, rhs->reg, dest);
        } else if(rhs->op == OP_VAR){
            gen_code("addq %%%s, %%%s", rhs->var_reg, dest);
        } else {
            lea_base_index(lhs->reg, rhs->reg, dest);
        }
    }
}

void gen_add_var_const(struct tree *node)
{
    debug("gen_add_var_const");

    struct tree *lhs = LEFT_CHILD(node);
    struct tree *rhs = RIGHT_CHILD(node);

    const char *dest = node->reg;

    struct tree *constnode = lhs->constant ? lhs : rhs;
    struct tree *varnode = lhs->op == OP_VAR ? lhs : rhs;

    long int value = tag_const(constnode->value);

    expect(varnode->var_reg, TYPE_NUMBER);

    lea_offset_base(value, varnode->var_reg, dest);
}

static void gen_sub_reg_const(long int value, const char *source, const char *dest, int tag_type)
{
    debug("gen_sub_reg_const");

    if(tag_type == TAGGED)
        value = tag_const(value);

    lea_offset_base(-value, source, dest);
}

static void gen_sub_const_reg(long int value, const char *source, const char *dest, int tag_type)
{
    debug("gen_sub_reg_const");

    if(tag_type == TAGGED)
        value = tag_const(value);

    gen_code("movq $%ld, %%%s", value, dest);
    gen_code("subq %%%s, %%%s", source, dest);
}

void gen_sub_var_const(struct tree *node, int tag_type)
{
    debug("gen_add_var_const");

    struct tree *lhs = LEFT_CHILD(node);
    struct tree *rhs = RIGHT_CHILD(node);

    const char *dest = node->reg;

    struct tree *constnode = lhs->constant ? lhs : rhs;
    struct tree *varnode = lhs->op == OP_VAR ? lhs : rhs;

    expect(varnode->var_reg, TYPE_NUMBER);

    if(lhs->op == OP_VAR) {
        gen_sub_reg_const(constnode->value, varnode->var_reg, dest, TAGGED);
    } else {
        gen_sub_const_reg(constnode->value, varnode->var_reg, dest, TAGGED);
    }
}


void gen_sub(struct tree *node, int tag_type)
{
    debug("gen_sub");

    struct tree *lhs = LEFT_CHILD(node);
    struct tree *rhs = RIGHT_CHILD(node);

    const char *dest = node->reg;

    if(lhs->constant && rhs->constant) {
        gen_code("TODO: upps, constant fold!");
    } else if(lhs->constant) {
        gen_sub_const_reg(lhs->value, rhs->reg, dest, tag_type);
    } else if(rhs->constant) {
        gen_sub_reg_const(rhs->value, lhs->reg, dest, tag_type);
    } else if(lhs->op == OP_VAR && rhs->op == OP_VAR){
        if(strcmp(lhs->name, rhs->name) == 0) {
            expect(lhs->var_reg, TYPE_NUMBER);
            gen_code("movq $0, %%%s", dest);
        } else {
            expect(lhs->var_reg, TYPE_NUMBER);
            expect(rhs->var_reg, TYPE_NUMBER);
            move(lhs->var_reg, dest);
            gen_code("subq %%%s, %%%s", rhs->var_reg, dest);
        }
    } else {
        if(lhs->op == OP_VAR) {
            move(lhs->var_reg, dest);
            gen_code("subq %%%s, %%%s", rhs->reg, dest);
        } else if(rhs->op == OP_VAR){
            gen_code("subq %%%s, %%%s", rhs->var_reg, dest);
        } else {
            move(lhs->reg, dest);
            gen_code("subq %%%s, %%%s", rhs->reg, dest);
        }
    }
}

/**
 * result is tagged if source reg is
 */
static void gen_mul_reg_const(const char *source, const char *dest, long int value)
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


void gen_mul(struct tree *node)
{
    debug("gen_mul");

    struct tree *lhs = LEFT_CHILD(node);
    struct tree *rhs = RIGHT_CHILD(node);

    const char *dest = node->reg;

    int op1 = lhs->op;
    int op2 = rhs->op;

    if(lhs->constant && rhs->constant) {
        gen_code("upps");
    }

    if(op1 == OP_VAR && op2 == OP_VAR) {
        expect(lhs->var_reg, TYPE_NUMBER);
        expect(rhs->var_reg, TYPE_NUMBER);
        move(lhs->var_reg, dest);
        untag_num_inplace(dest);
        gen_code("imulq %%%s, %%%s", rhs->var_reg, dest);
        untag_num_inplace(dest);
    } else if(op1 == OP_VAR) {
        expect_num(rhs);
        if(rhs->constant) {
            gen_mul_reg_const(lhs->var_reg, dest, rhs->value);
            untag_num_inplace(dest);
        } else {
            move(rhs->reg, dest);
            gen_code("imulq %%%s, %%%s", lhs->var_reg, dest);
            untag_num_inplace(dest);
        }
    } else if(op2 == OP_VAR) {
        expect_num(rhs);
        if(lhs->constant) {
            gen_mul_reg_const(rhs->var_reg, dest, lhs->value);
            untag_num_inplace(dest);
        } else {
            gen_code("imulq %%%s, %%%s", rhs->var_reg, dest);
            untag_num_inplace(dest);
        }
    } else {
        if(lhs->constant)
            gen_mul_reg_const(rhs->reg, dest, lhs->value);
        else if(rhs->constant)
            gen_mul_reg_const(lhs->reg, dest, rhs->value);
        else
            gen_code("imulq %%%s, %%%s", rhs->reg, dest);
    }
}

/**
 *  result is untagged
 */
void gen_mul_var_const(struct tree *node)
{
    debug("gen_mul_var_const");

    struct tree *lhs = LEFT_CHILD(node);
    struct tree *rhs = RIGHT_CHILD(node);

    const char *dest = node->reg;

    struct tree *constnode = lhs->constant ? lhs : rhs;
    struct tree *varnode = lhs->op == OP_VAR ? lhs : rhs;

    expect(varnode->var_reg, TYPE_NUMBER);
    gen_mul_reg_const(varnode->var_reg, dest, constnode->value);
    untag_num_inplace(dest);
}

static void gen_eq_const(long int value, const char *source, const char *dest)
{
    debug("gen_eq_const");

    value = tag_const(value);

    gen_code("movq $2, %%%s", temp_reg);
    gen_code("cmp $%ld, %%%s", value, source);
    gen_code("leaq 0(, 1), %%%s", dest);
    gen_code("cmovz %%%s, %%%s", temp_reg, dest);
}

static void gen_eq_reg_reg(const char *s1, const char *s2, const char *dest)
{
    debug("gen_eq_reg_reg");

    gen_code("movq $2, %%%s", temp_reg);
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
        gen_eq_reg_reg(lhs->var_reg, rhs->var_reg, dest);
    } else if(lhs->op == OP_VAR) {
        if(rhs->constant) {
            gen_eq_const(rhs->value, lhs->var_reg, dest);
        } else {
            gen_eq_reg_reg(lhs->var_reg, rhs->reg, dest);
        }
    } else if(rhs->op == OP_VAR) {
        if(lhs->constant) {
            gen_eq_const(lhs->value, rhs->var_reg, dest);
        } else {
            gen_eq_reg_reg(rhs->var_reg, lhs->reg, dest);
        }
    } else {
        if(lhs->constant)
            gen_eq_const(lhs->value, rhs->reg, dest);
        else if(rhs->constant)
            gen_eq_const(rhs->value, lhs->reg, dest);
        else
            gen_eq_reg_reg(lhs->reg, rhs->reg, dest);
    }
}

static void gen_lt_reg_reg(const char *s1, const char *s2, const char *dest)
{
    debug("gen_lt_reg_reg");

    gen_code("movq $2, %%%s", temp_reg);
    gen_code("cmp %%%s, %%%s", s1, s2);
    gen_code("leaq 0(, 1), %%%s", dest);
    gen_code("cmovg %%%s, %%%s", temp_reg, dest);
}

static void gen_lt_const_reg(long int value, const char *source, const char *dest)
{
    debug("gen_lt_const");

    gen_code("movq $2, %%%s", temp_reg);
    gen_code("cmp $%ld, %%%s", tag_const(value), source);
    gen_code("leaq 0(, 1), %%%s", dest);
    gen_code("cmovg %%%s, %%%s", temp_reg, dest);
}

static void gen_lt_reg_const(long int value, const char *source, const char *dest)
{
    debug("gen_lt_const");

    gen_code("movq $2, %%%s", temp_reg);
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
        expect_num(lhs);
        expect_num(rhs);
        gen_lt_reg_reg(lhs->var_reg, rhs->var_reg, dest);
    } else if(lhs->op == OP_VAR) {
        expect_num(lhs);
        if(rhs->constant) {
            gen_lt_reg_const(rhs->value, lhs->var_reg, dest);
        } else {
            gen_lt_reg_reg(lhs->var_reg, rhs->reg, dest);
        }
    } else if(rhs->op == OP_VAR) {
        expect_num(rhs);
        if(lhs->constant) {
            gen_lt_const_reg(lhs->value, rhs->var_reg, dest);
        } else {
            gen_lt_reg_reg(lhs->reg, rhs->var_reg, dest);
        }
    } else {
        if(lhs->constant)
            gen_lt_const_reg(lhs->value, rhs->reg, dest);
        else if(rhs->constant)
            gen_lt_reg_const(rhs->value, lhs->reg, dest);
        else
            gen_lt_reg_reg(lhs->reg, rhs->reg, dest);
    }
}

void gen_isnum(struct tree *node)
{
    debug("gen_isnum");

    struct tree *lhs = LEFT_CHILD(node);
    const char *dest = node->reg;

    if(lhs->op == OP_VAR) {
        move(lhs->var_reg, dest);
        gen_code("xorq $1, %rax");
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
        source = lhs->var_reg;
    } else {
        source = lhs->reg;
    }

    gen_code("movq $0, %%%s", temp_reg);

    move(source, dest);
    gen_code("andq $3, %%%s", dest);
    gen_code("cmpq $1, %%%s", dest);
    gen_code("lea 0(,1), %%%s", dest);
    gen_code("cmovz %%%s, %%%s", temp_reg, dest);
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


/**
 * Constructs a list from a const, var or expression as lhs and rhs. The
 * resulting address is untagged.
 */
void gen_list_simple(struct tree *node)
{
    debug("gen_list");

    struct tree *lhs = LEFT_CHILD(node);
    struct tree *rhs = RIGHT_CHILD(node);

    const char *dest = node->reg;

    if(lhs->constant)
        set_head_const("r15", lhs->value);
    else if(lhs->op == OP_VAR)
        set_head_reg("r15", lhs->var_reg);
    else
        set_head_reg("r15", lhs->reg);

    if(rhs->constant)
        set_tail_const("r15", rhs->value);
    else if(rhs->op == OP_VAR)
        set_tail_reg("r15", rhs->var_reg);
    else
        set_tail_reg("r15", rhs->reg);

    // return cell address and advance list pointer in r15
    move("r15", dest);
    gen_code("addq $16, %%%s", "r15");
}

void gen_list_prepend(struct tree *node)
{
    debug("gen_list_expr_list");

    struct tree *lhs = LEFT_CHILD(node);
    struct tree *listnode = RIGHT_CHILD(node);

    const char *dest = node->reg;

    if(lhs->constant)
        set_head_const("r15", lhs->value);
    else if(lhs->op == OP_VAR)
        set_head_reg("r15", lhs->var_reg);
    else
        set_head_reg("r15", lhs->reg);

    // set tail and tag it
    gen_code("movq %%%s, 8(%%%s)", listnode->reg, "r15");
    gen_code("addq $1, 8(%%%s)", "r15");

    // return cell address and advance list pointer in r15
    move("r15", dest);
    gen_code("addq $16, %%%s", "r15");
}
