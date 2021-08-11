#include "../include/codegen.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//max length of a register's string representation
#define REGSTRLEN 3

//maximum length a 64 bit int can be when converted to str
#define MAXINTSTRLEN 20

struct CGState {
    char *code;
    size_t code_len;
    size_t code_capacity;
    size_t num_regs;
    char *regs_free;
    char **reg_names;
    char **op_names;
    SymTable *table;
};

enum {
    CG_ADD,
    CG_SUB,
    CG_MUL,
    CG_DIV,
    CG_CMP,
    CG_LT,
    CG_LTE,
    CG_GT,
    CG_GTE
};

typedef struct CGState CGState;

void state_alloc_atleast(CGState *s, size_t bytes) {
    //sanity check
    if (bytes <= 0) {
        fprintf(stderr, "WARNING: state_alloc_atleast() was called with bytes <= 0. This is almost certainly a bug\n");
    }
    //if already enough bytes
    if (s->code_capacity - s->code_len > bytes) {
        return;
    }
    //allocate double the memory until we have enough
    size_t to_alloc = s->code_capacity * 2;
    while (to_alloc - s->code_len < bytes) {
        to_alloc *= 2;
    }
    s->code = realloc(s->code, to_alloc);
    s->code_capacity = to_alloc;
}

void state_init(CGState *s, size_t num_regs, char **reg_names, char **op_names, struct SymTable *table) {
    s->regs_free = malloc(num_regs);
    s->num_regs = num_regs;
    s->reg_names = reg_names;
    for (size_t i = 0; i < num_regs; i++) {
        s->regs_free[i] = 0;
    }
    s->code = malloc(8);
    s->code_len = 0;
    s->code_capacity = 8;
    s->table = table;
    s->op_names = op_names;
}

void state_free(CGState *s) {
    free(s->regs_free);
}

int reg_alloc(CGState *state) {
    for (size_t i = 0; i < 4; i++) {
        if (state->regs_free[i] == 0) {
            state->regs_free[i] = 1;
            return i;
        }
    }
    return -1;
}

void reg_free(CGState *state, int reg) {
    state->regs_free[reg] = 0;
}

int cgmathop(int rega, int regb, int op, CGState *state) {
    state_alloc_atleast(state, 12 + REGSTRLEN * 3);
    state->code_len += sprintf(state->code + state->code_len, "\t%s\t%s, %s -> %s\n", state->op_names[op], state->reg_names[rega], state->reg_names[regb], state->reg_names[rega]);
    reg_free(state, regb);
    return rega;
}

int cgloadint(int val, CGState *state) {
    int reg = reg_alloc(state);
    state_alloc_atleast(state, 11 + REGSTRLEN + MAXINTSTRLEN);
    state->code_len += sprintf(state->code + state->code_len, "\tmovl\t%d -> %s\n", val, state->reg_names[reg]);
    return reg;
}

void cgprintint(CGState *state, int reg) {
    state_alloc_atleast(state, 6 + REGSTRLEN);
    state->code_len += sprintf(state->code + state->code_len, "\tout\t%s\n", state->reg_names[reg]);
}

//void cgdefglob(CGState *state) {
//    state_alloc_atleast()
//}

void cgstoreglob(CGState *state, int reg, char *ident) {
    state_alloc_atleast(state, 12 + REGSTRLEN + MAXINTSTRLEN);
    state->code_len += sprintf(state->code + state->code_len, "\tmovra\t%s -> %s\n", state->reg_names[reg], ident);
    reg_free(state, reg);
}

int cgloadglob(CGState *state, char *ident) {
    int reg = reg_alloc(state);
    state_alloc_atleast(state, 12 + REGSTRLEN + MAXINTSTRLEN);
    state->code_len += sprintf(state->code + state->code_len, "\tmovar\t%s -> %s\n", ident, state->reg_names[reg]);
    return reg;
}

char *sym_find(SymTable *table, size_t ident) {
    for (size_t i = 0; i < table->pos; i++) {
        if (table->symbols[i].ident == ident) {
            return table->symbols[i].s;
        }
    }
    return NULL;
}

int gen_ast(AST *ast, CGState *state, int reg) {
    int *ch_regs;
    if (ast->children_n > 0) {
        ch_regs = malloc(sizeof(int) * ast->children_n);
        for (size_t i = 0; i < ast->children_n; i++) {
           ch_regs[i] = gen_ast(ast->children[i], state, i > 0 ? ch_regs[i - 1] : -1);
        }
    }

    int res = -1;

    switch (ast->type) {
        case AST_PROG:   break; //already calculated all the children so leave
        case AST_ADD:    res = cgmathop(ch_regs[0], ch_regs[1], CG_ADD, state); break;
        case AST_SUB:    res = cgmathop(ch_regs[0], ch_regs[1], CG_SUB, state); break;
        case AST_MUL:    res = cgmathop(ch_regs[0], ch_regs[1], CG_MUL, state); break;
        case AST_DIV:    res = cgmathop(ch_regs[0], ch_regs[1], CG_DIV, state); break;
        case AST_COMPARE:res = cgmathop(ch_regs[0], ch_regs[1], CG_CMP, state); break;
        case AST_GT:     res = cgmathop(ch_regs[0], ch_regs[1], CG_GT, state); break;
        case AST_GTE:    res = cgmathop(ch_regs[0], ch_regs[1], CG_GTE, state); break;
        case AST_LT:     res = cgmathop(ch_regs[0], ch_regs[1], CG_LT, state); break;
        case AST_LTE:    res = cgmathop(ch_regs[0], ch_regs[1], CG_LTE, state); break;
        case AST_INT_LIT:res = cgloadint(ast->i, state); break;
        case AST_IDENT:  res = cgloadglob(state, sym_find(state->table, ast->i)); break;
        case AST_LVIDENT:cgstoreglob(state, reg, sym_find(state->table, ast->i)); break;
        case AST_ASSIGN: res = ch_regs[0]; break;
        case AST_KPRINT: cgprintint(state, ch_regs[0]); break;
        default:
            fprintf(stderr, "Error: unrecognised token '%s'\n", asttypetostr(ast->type));
            exit(0);
            break;
    }

    if (ast->children_n > 0) {
        free(ch_regs);
    }
    return res;
}

Error cg_gen(AST *ast, char **code, SymTable *table) {
    char *reg_names[] = {"r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"};
    char *op_names[]  = {"add", "sub", "mul", "div", "cmp", "lt", "lte", "gt", "gte"};
    const char *preamble = "_start:\n";
    const char *postamble = "\tret\n";

    size_t decl_len = 128; //fuck off
    size_t pre_len  = strlen(preamble);
    size_t post_len = strlen(postamble);
    
    CGState state;
    state_init(&state, 16, reg_names, op_names, table);

    state_alloc_atleast(&state, pre_len + decl_len);

    for (uint32_t i = 0; i < table->pos; i++) {
        state.code_len += sprintf(state.code + state.code_len, "%s: dd 0\n", table->symbols[i].s);
    }
    
    strcpy(state.code + state.code_len, preamble);
    state.code_len += pre_len;
    
    gen_ast(ast, &state, -1);
    
    state_alloc_atleast(&state, post_len);
    strcpy(state.code + state.code_len, postamble);
    state.code_len += post_len;
    
    state_free(&state);

    *code = state.code;

    return (Error){0, NULL};
}

