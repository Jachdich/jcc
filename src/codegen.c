#include "../include/codegen.h"
#include <stdlib.h>
#include <stdio.h>

struct CGState {
    char *curr_code;
    size_t code_len;
    size_t code_capacity;
    char regs_free[4];
};

typedef struct CGState CGState;

void state_init(CGState *s) {
    for (size_t i = 0; i < 4; i++) {
        s->regs_free[i] = 0;
    }
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

int cgadd(int rega, int regb, CGState *state) {
    printf("add r%d, r%d\n", rega, regb);
    reg_free(state, regb);
    return rega;
}

int cgsub(int rega, int regb, CGState *state) {
    printf("sub r%d, r%d\n", rega, regb);
    reg_free(state, regb);
    return rega;
}

int cgmul(int rega, int regb, CGState *state) {
    printf("mul r%d, r%d\n", rega, regb);
    reg_free(state, regb);
    return rega;
}

int cgdiv(int rega, int regb, CGState *state) {
    printf("div r%d, r%d\n", rega, regb);
    reg_free(state, regb);
    return rega;
}

int cgload(int val, CGState *state) {
    int reg = reg_alloc(state);
    printf("mov r%d, %d\n", reg, val);
    return reg;
}

int gen_ast(AST *ast, CGState *state) {
    int *ch_regs;
    if (ast->children_n > 0) {
    ch_regs = malloc(sizeof(int) * ast->children_n);
        for (size_t i = 0; i < ast->children_n; i++) {
           ch_regs[i] = gen_ast(ast->children[i], state);
        }
    }

    int res;

    switch (ast->tok->type) {
        case TOK_ADD: res = cgadd(ch_regs[0], ch_regs[1], state); break;
        case TOK_SUB: res = cgsub(ch_regs[0], ch_regs[1], state); break;
        case TOK_MUL: res = cgmul(ch_regs[0], ch_regs[1], state); break;
        case TOK_DIV: res = cgdiv(ch_regs[0], ch_regs[1], state); break;
        case TOK_INT: res = cgload(ast->tok->i, state); break;
        default: break;
    }

    if (ast->children_n > 0) {
        free(ch_regs);
    }
    return res;
}

Error cg_gen(AST *ast, char **code) {
    CGState state;
    state_init(&state);
    int reg = gen_ast(ast, &state);
    return (Error){0, NULL};
}