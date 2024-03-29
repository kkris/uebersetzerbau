#include "codegen.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>

#include "symbol.h"

char* registers[] = {"rdi", "rax", "rsi", "rdx", "rcx", "r8", "r9", "r10"};

const char *heap_ptr = "r15";
const char *temp_reg = "r11";
const char *frame_ptr = "r11";

// global variable which tracks the just tagged register
char *just_tagged = NULL;
char *just_untagged = NULL;

int cur_var_reg = 0;
const char *var_reg0 = "r12";
const char *var_reg1 = "r13";


static char *type_checked_vars[10] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};

static void reset_state()
{
    just_tagged = NULL;
    just_untagged = NULL;

    int i;
    for(i = 0; i < 10; i++) {
        type_checked_vars[i] = NULL;
    }
}

static void maybe_force_tag_or_untag();

static void debug(const char *msg, ...)
{
    return;

    va_list ap;

    va_start(ap, msg);
    vfprintf(stderr, msg, ap);
    fprintf(stderr, "\n");
    va_end(ap);
}

char *alloc_reg(const char *prev, int reuse)
{
    if(prev == NULL)
        return strdup("rdi");

    unsigned int i = 0;
    for(; i < sizeof(registers) / sizeof(registers[0]) - 1; i++) {
        if(strcmp(prev, registers[i]) == 0) {
            if(reuse == 1)
                return strdup(registers[i]);
            else
                return strdup(registers[i + 1]);
        }
    }

    return NULL;
}

long int tag_const(long int value)
{
    return value << 1;
}

char *alloc_var_reg(struct tree *parent, struct tree *expr)
{
    if(expr->op == OP_VAR) {
        return strdup(expr->symbol->reg); /* alias variables */
    }

    return alloc_reg(parent->reg, 1);
}

/* in each nodes symbol table set the register for a variable */
void set_symbol_reg_children(struct tree *node, char *name, const char *reg)
{
    if(node == NULL)
        return;

    struct symbol *sym = symbol_find(node->symbol, name);
    if(sym != NULL) {
        sym->reg = strdup(reg);
        sym->orig_reg = strdup(reg);
    }

    set_symbol_reg_children(LEFT_CHILD(node), name, reg);
    set_symbol_reg_children(RIGHT_CHILD(node), name, reg);
}

void set_captured(struct tree *node, const char *name, int offset)
{
    if(node == NULL)
        return;

    struct symbol *current = node->symbol;
    while(current != NULL) {
        if(strcmp(current->name, name) == 0) {
            if(current->captured != 1) {
                current->outer = 1;
            }
            current->captured = 1;
            current->offset = offset;
            current->reg = strdup(var_reg0);
        }
        current = current->next;
    }

    set_captured(LEFT_CHILD(node), name, offset);
    set_captured(RIGHT_CHILD(node), name, offset);
}

/* in each nodes symbol table set all variables -except this one- as captured */
void mark_other_symbols_as_captured(struct tree *node, char *except)
{
    if(node == NULL)
        return;

    int offset = 0;

    struct symbol *current = node->symbol;
    while(current != NULL) {
        if(strcmp(current->name, except) == 0) {

        } else if(current->type != SYMBOL_TYPE_FUN) {
            current->captured = 1;
            current->offset = offset;

            set_captured(LEFT_CHILD(node), current->name, offset);
            set_captured(RIGHT_CHILD(node), current->name, offset);

            offset = offset + 1;
        }

        current = current->next;
    }
}


static void gen_code(const char *code, ...)
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
    gen_code("push %%%s", var_reg0);
    gen_code("push %%%s", var_reg1);
}

static void move(const char *source, const char *dest)
{
    if(strcmp(source, dest) == 0)
        return; // nop

    gen_code("movq %%%s, %%%s", source, dest);
}

void assign_const(long int value, const char *var_reg)
{
    move_const(tag_const(value), var_reg);

    int i;
    for(i = 0; i < 10; i++) {
        if(type_checked_vars[i] == NULL) break;
        if(strcmp(var_reg, type_checked_vars[i]) == 0)
            return;
    }

    type_checked_vars[i] = strdup(var_reg);
}

void reg_is_number(const char *var_reg)
{
    int i;
    for(i = 0; i < 10; i++) {
        if(type_checked_vars[i] == NULL) break;
        if(strcmp(var_reg, type_checked_vars[i]) == 0)
            return;
    }

    type_checked_vars[i] = strdup(var_reg);
}

void move_const(long int value, const char *dest)
{
    gen_code("movq $%ld, %%%s", value, dest);
}

static void lea_const_reg(long int offset, const char *base, const char *dest)
{
    if(offset == 0)
        move(base, dest);
    else
        gen_code("leaq %ld(%%%s), %%%s", offset, base, dest);
}

static void lea_reg_reg(const char *base, const char *index, const char *dest)
{
    gen_code("leaq (%%%s, %%%s), %%%s", base, index, dest);
}

void raise_signal()
{
    gen_code("jmp raisesig");
}

int check_var(const char *var_reg)
{
    debug("expect");

    int i;
    for(i = 0; i < 10; i++) {
        if(type_checked_vars[i] == NULL) break;
        if(strcmp(var_reg, type_checked_vars[i]) == 0)
            return 0;
    }

    type_checked_vars[i] = strdup(var_reg);

    return 1;
}

void expect_num(struct tree *node)
{
    debug("expect_num");

    if(node->op != OP_VAR || check_var(node->reg)) {
        gen_code("testq $1, %%%s", node->reg);
        gen_code("jnz raisesig");
    }
}

void expect_list(struct tree *node)
{
    debug("expect_list");

    if(node->op != OP_VAR || check_var(node->reg)) {
        gen_code("testq $1, %%%s", node->reg);
        gen_code("jz raisesig");
        gen_code("testq $2, %%%s", node->reg);
        gen_code("jnz raisesig");
    }
}

void expect_closure(struct tree *node)
{
    debug("expect_closure");
    gen_code("testq $1, %%%s", node->reg);
    gen_code("jz raisesig");
    gen_code("testq $2, %%%s", node->reg);
    gen_code("jz raisesig");
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
static void maybe_force_tag_or_untag()
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
    just_untagged = NULL;
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
    just_tagged = NULL;
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

void untag_closure_inplace(const char *reg)
{
    debug("untag_closure_inplace");

    gen_code("subq $3, %%%s", reg);
}

void ret(struct tree *node, int tag_type, int type)
{
    debug("ret");

    if(tag_type == TAGGED) {
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
        }
    }

    gen_code("pop %%%s", var_reg1);
    gen_code("pop %%%s", var_reg0);

    gen_code("ret");

    reset_state();
}

void load_var(struct tree *node)
{
    if(node->symbol->captured) {
        int stack_offset = node->symbol->offset * 8;

        const char *reg;
        if(cur_var_reg == 0)
            reg = var_reg0;
        else
            reg = var_reg1;

        gen_code("movq %d(%%%s), %%%s\t# load %s", stack_offset, "rsp", reg, node->symbol->name);
        cur_var_reg = (cur_var_reg + 1) % 2;

        node->reg = strdup(reg);
    }
}

void gen_not(struct tree *node)
{
    debug("gen_not");

    struct tree *lhs = LEFT_CHILD(node);

    const char *dest = node->reg;

    move(lhs->reg, dest);
    gen_code("xorq $2, %%%s", dest);
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

void gen_add(struct tree *node)
{
    debug("gen_add");

    struct tree *lhs = LEFT_CHILD(node);
    struct tree *rhs = RIGHT_CHILD(node);

    const char *dest = node->reg;

    if(lhs->constant) {
        lea_const_reg(tag_const(lhs->value), rhs->reg, dest);
    } else if(rhs->constant) {
        lea_const_reg(tag_const(rhs->value), lhs->reg, dest);
    } else {
        lea_reg_reg(lhs->reg, rhs->reg, dest);
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
        lea_const_reg(-tag_const(rhs->value), lhs->reg, dest);
    } else {
        gen_sub_reg_reg(lhs->reg, rhs->reg, dest);
    }
}

static void gen_mul_const_reg(long int value, const char *source, const char *dest)
{
    debug("gen_mul_reg_const");

    switch(value) {
    case 2:
        gen_code("leaq (, %%%s, 2), %%%s", source, dest); break;
    case 3:
        gen_code("leaq (%%%s, %%%s, 2), %%%s", source, source, dest); break;
    case 4:
        gen_code("leaq (, %%%s, 4), %%%s", source, dest); break;
    case 5:
        gen_code("leaq (%%%s, %%%s, 4), %%%s", source, source, dest); break;
    case 8:
        gen_code("leaq (, %%%s, 8), %%%s", source, dest); break;
    case 9:
        gen_code("leaq (%%%s, %%%s, 8), %%%s", source, source, dest); break;
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

    if(lhs->constant) {
        move_const(2, dest);
    } else {
        move(lhs->reg, dest);
        gen_code("andq $1, %%%s", dest);
        gen_code("xorq $1, %%%s", dest);
    }
}

void gen_islist(struct tree *node)
{
    debug("gen_islist");

    struct tree *lhs = LEFT_CHILD(node);

    const char *dest = node->reg;

    move_const(2, temp_reg);
    move(lhs->reg, dest);
    gen_code("andq $3, %%%s", dest);
    gen_code("cmpq $1, %%%s", dest);
    gen_code("lea 0(,1), %%%s", dest);
    gen_code("cmovz %%%s, %%%s", temp_reg, dest);
}

void gen_isfun(struct tree *node)
{
    debug("gen_islist");

    struct tree *lhs = LEFT_CHILD(node);

    const char *dest = node->reg;

    move_const(2, temp_reg);
    move(lhs->reg, dest);
    gen_code("andq $3, %%%s", dest);
    gen_code("cmpq $3, %%%s", dest);
    gen_code("lea 0(,1), %%%s", dest);
    gen_code("cmovz %%%s, %%%s", temp_reg, dest);
}


void gen_head(struct tree *node, int tag_type)
{
    debug("gen_head");

    struct tree *lhs = LEFT_CHILD(node);
    const char *dest = node->reg;

    if(tag_type == TAGGED) {
        expect_list(lhs);
        move(lhs->reg, dest);
        gen_code("movq -1(%%%s), %%%s", dest, dest);
    } else {
        gen_code("movq (%%%s), %%%s", lhs->reg, dest);
    }
}

void gen_tail(struct tree *node, int tag_type)
{
    debug("gen_tail");

    struct tree *lhs = LEFT_CHILD(node);
    const char *dest = node->reg;

    if(tag_type == TAGGED) {
        expect_list(lhs);
        move(lhs->reg, dest);
        gen_code("movq 7(%%%s), %%%s", dest, dest);
    } else {
        gen_code("movq 8(%%%s), %%%s", lhs->reg, dest);
    }
}

void gen_list(struct tree *node)
{
    debug("gen_list");

    struct tree *lhs = LEFT_CHILD(node);
    struct tree *rhs = RIGHT_CHILD(node);

    const char *dest = node->reg;

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

void gen_if(struct tree *node)
{
    debug("gen_if");

    struct tree *pred = LEFT_CHILD(node);
    struct label_pair *labels = (struct label_pair*)node->data;

    if(pred->constant) {
        move_const(tag_const(pred->value), node->reg);
    }

    gen_code("cmpq $0, %%%s", pred->reg);
    gen_code("jz .%s", labels->else_label);
}


void gen_ifthen(struct tree *node)
{
    debug("gen_ifthen");

    struct tree *then = RIGHT_CHILD(node);
    struct label_pair *labels = (struct label_pair*)node->data;

    if(then->constant) {
        move_const(tag_const(then->value), node->reg);
    }

    move(then->reg, node->reg);


    gen_code("jmp .%s", labels->epilog_label);
    printf(".%s:\n", labels->else_label);
}

void gen_ifthenelse(struct tree *node)
{
    debug("gen_ifthenelse");

    struct tree *elsenode = RIGHT_CHILD(node);
    struct label_pair *labels = (struct label_pair*)node->data;

    if(elsenode->constant) {
        move_const(tag_const(elsenode->value), node->reg);
    }

    move(elsenode->reg, node->reg);


    printf(".%s:\n", labels->epilog_label);
}

void gen_let(struct tree *node)
{
    debug("gen_let");

    struct tree *lhs = LEFT_CHILD(node);
    struct tree *rhs = RIGHT_CHILD(node);

    if(lhs->constant) {
        move_const(tag_const(lhs->value), lhs->reg);
    } else {
        move(rhs->reg, node->reg);
    }
}

void gen_call(struct tree *node)
{
    debug("gen_call");

    struct tree *fun = LEFT_CHILD(node);
    struct tree *param = RIGHT_CHILD(node);

    int i;
    for(i = 2; i < sizeof(registers) / sizeof(registers[0]); i++) {
        gen_code("push %%%s", registers[i]);
    }

    gen_code("push %%%s", "rdi");

    if(param->constant)
        move_const(tag_const(param->value), "rdi");
    else
        move(param->reg, "rdi");

    gen_code("call %s", fun->symbol->name);
    gen_code("pop %%%s", "rdi");

    for(i = sizeof(registers) / sizeof(registers[0]) - 1; i > 1; i--) {
        gen_code("pop %%%s", registers[i]);
    }

    move("rax", node->reg);
}

void make_closure(struct tree *node)
{
    debug("make_closure");

    struct closure_data *data = (struct closure_data*)node->data;

    gen_code("lea (_closure%d), %%%s", data->num, node->reg);

    // make closure cell
    gen_code("movq %%%s, (%%%s)", node->reg, heap_ptr);
    gen_code("movq %%%s, 8(%%%s)", frame_ptr, heap_ptr);
    gen_code("movq %%%s, %%%s", heap_ptr, node->reg);
    gen_code("addq $3, %%%s", node->reg); // tag as closure
    gen_code("addq $16, %%%s", heap_ptr);
}

static void make_frame()
{
    gen_code("movq %%%s, (%%%s)", frame_ptr, heap_ptr);
    move(heap_ptr, frame_ptr);
    gen_code("addq $16, %%%s", heap_ptr);

}

static void save_captured_vars_in_frames(struct tree *node)
{
    gen_code("# save captured variables in frame");
    struct symbol *sym = node->symbol;
    while(sym != NULL) {
        if(sym->captured) {
            make_frame();
            if(sym->outer) {
                gen_code("movq %d(%%%s), %%%s", (sym->offset) * 8, "rsp", var_reg0);
                gen_code("movq %%%s, 8(%%%s)", var_reg0, frame_ptr);
            } else {
                gen_code("movq %%%s, 8(%%%s)", sym->orig_reg, frame_ptr);
            }
        }
        sym = sym->next;
    }
}

static void load_captured_onto_stack(struct tree *node)
{
    int first = 1;
    struct symbol *sym = node->symbol;

    while(sym != NULL) {
        if(sym->captured) {
            if(!first) {
                gen_code("movq (%%%s), %%%s", frame_ptr, frame_ptr);
            } else {
                gen_code("# load captured variables into scope");
            }

            gen_code("push 8(%%%s)", frame_ptr);

            if(first) {
                first = 0;
            }
        }
        sym = sym->next;
    }
}

static void unload_captured_from_stack(struct tree *node)
{
    struct closure_data *data = (struct closure_data*)node->data;

    int first = 1;
    struct symbol *sym = node->symbol;
    while(sym != NULL) {
        if(sym->captured) {
            gen_code("pop %%%s", temp_reg);
        }
        sym = sym->next;
    }
}

void gen_lambda_prolog(struct tree *node)
{
    debug("gen_lambda_prolog");

    gen_code("# save captured variables in frame");
    gen_code("movq $0, %%%s", frame_ptr);

    save_captured_vars_in_frames(node);

    struct closure_data *data = (struct closure_data*)node->data;

    gen_code("# make closure");
    make_closure(node);
    gen_code("jmp .return_closure_%d", data->num);
    gen_code("_closure%d:", data->num);

    load_captured_onto_stack(node);
}

void gen_lambda_epilog(struct tree *node)
{
    debug("gen_lambda_epilog");

    struct closure_data *data = (struct closure_data*)node->data;
    struct tree *body = RIGHT_CHILD(node);

    if(body->op == OP_VAR)
        move(body->reg, LEFT_CHILD(node)->reg);
    else if(body->constant)
        move_const(tag_const(body->value), LEFT_CHILD(node)->reg);

    unload_captured_from_stack(node);

    move(LEFT_CHILD(node)->reg, "rax");
    gen_code("ret");
    printf(".return_closure_%d:\n", data->num);
    move(LEFT_CHILD(node)->reg, node->reg);
}

void gen_call_closure(struct tree *node)
{
    debug("gen_call_closure");

    struct tree *fun = LEFT_CHILD(node);
    struct tree *param = RIGHT_CHILD(node);

    gen_code("movq 8(%%%s), %%%s\t# pass environment", fun->reg, frame_ptr);

    int i;
    for(i = 2; i < sizeof(registers) / sizeof(registers[0]); i++) {
        if(i == 2) {
            gen_code("push %%%s\t# save ctx", registers[i]);
        } else {
            gen_code("push %%%s", registers[i]);
        }
    }

    gen_code("push %%%s", "rdi");

    move(fun->reg, node->reg);

    if(param->constant)
        move_const(tag_const(param->value), "rdi");
    else
        move(param->reg, "rdi");

    gen_code("call *(%%%s)\t\t# call closure", node->reg);

    gen_code("pop %%%s", "rdi");

    for(i = sizeof(registers) / sizeof(registers[0]) - 1; i > 1; i--) {
        if (i == (sizeof(registers) / sizeof(registers[0]) - 1)) {
            gen_code("pop %%%s\t# restore ctx", registers[i]);
        } else {
            gen_code("pop %%%s", registers[i]);
        }
    }

    move("rax", node->reg);
}

void func_to_word(struct tree *node)
{
    gen_code("lea (%s), %%%s", node->symbol->name, node->reg);
    gen_code("movq %%%s, (%%%s)", node->reg, heap_ptr);
    gen_code("movq %%%s, 8(%%%s)", frame_ptr, heap_ptr);
    gen_code("movq %%%s, %%%s", heap_ptr, node->reg);
    gen_code("addq $3, %%%s", node->reg); // tag as closure
    gen_code("addq $16, %%%s", heap_ptr);

}
