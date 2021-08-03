#include "../include/codegen.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//max length of a register's string representation
#define REGSTRLEN 8

//maximum length a 64 bit int can be when converted to str
#define MAXINTSTRLEN 20

struct CGState {
    char *code;
    size_t code_len;
    size_t code_capacity;
    size_t num_regs;
    char *regs_free;
    char **reg_names;
};

typedef struct CGState CGState;

void state_alloc_atleast(CGState *s, size_t bytes) {
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

void state_init(CGState *s, size_t num_regs, char **reg_names) {
    s->regs_free = malloc(num_regs);
    s->num_regs = num_regs;
    s->reg_names = reg_names;
    for (size_t i = 0; i < num_regs; i++) {
        s->regs_free[i] = 0;
    }
    s->code = malloc(8);
    s->code_len = 0;
    s->code_capacity = 8;
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

int cgadd(int rega, int regb, CGState *state) {
    state_alloc_atleast(state, 8 + REGSTRLEN * 2);
    state->code_len += sprintf(state->code + state->code_len, "\tadd\t%s, %s\n", state->reg_names[regb], state->reg_names[rega]);
    reg_free(state, rega);
    return regb;
}

int cgsub(int rega, int regb, CGState *state) {
    state_alloc_atleast(state, 8 + REGSTRLEN * 2);
    state->code_len += sprintf(state->code + state->code_len, "\tsub\t%s, %s\n", state->reg_names[rega], state->reg_names[regb]);
    reg_free(state, regb);
    return rega;
}

int cgmul(int rega, int regb, CGState *state) {
    state_alloc_atleast(state, 9 + REGSTRLEN * 2);
    state->code_len += sprintf(state->code + state->code_len, "\timul\t%s, %s\n", state->reg_names[regb], state->reg_names[rega]);
    reg_free(state, rega);
    return regb;
}

int cgdiv(int rega, int regb, CGState *state) {
    state_alloc_atleast(state, 35 + REGSTRLEN * 3);
    state->code_len += sprintf(state->code + state->code_len, "\tmov rax, %s\n", state->reg_names[rega]);
    state->code_len += sprintf(state->code + state->code_len, "\tcqo\n");
    state->code_len += sprintf(state->code + state->code_len, "\tidiv\t%s\n", state->reg_names[regb]);
    state->code_len += sprintf(state->code + state->code_len, "\tmov %s, rax\n", state->reg_names[rega]);
    reg_free(state, regb);
    return rega;
}

int cgload(int val, CGState *state) {
    int reg = reg_alloc(state);
    state_alloc_atleast(state, 8 + REGSTRLEN + MAXINTSTRLEN);
    state->code_len += sprintf(state->code + state->code_len, "\tmov\t%s, %d\n", state->reg_names[reg], val);
    return reg;
}

void cgprintint(CGState *state, int reg) {
    state_alloc_atleast(state, 26 + REGSTRLEN);
    state->code_len += sprintf(state->code + state->code_len, "\tmov\trdi, %s\n", state->reg_names[reg]);
    state->code_len += sprintf(state->code + state->code_len, "\tcall\tprintint\n");
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

    switch (ast->type) {
        case AST_ADD:    res = cgadd(ch_regs[0], ch_regs[1], state); break;
        case AST_SUB:    res = cgsub(ch_regs[0], ch_regs[1], state); break;
        case AST_MUL:    res = cgmul(ch_regs[0], ch_regs[1], state); break;
        case AST_DIV:    res = cgdiv(ch_regs[0], ch_regs[1], state); break;
        case AST_INT_LIT:res = cgload(ast->i, state); break;
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

Error cg_gen(AST *ast, char **code) {
    char *reg_names[] = {"r8", "r9", "r10", "r11"};
    const char *preamble = "default rel\n"
        "extern printf\n"
        "section .rodata\n"
        "\tformat db \"%d\", 10, 0\n"
        "section .text\n"
        "global main\n"
        "printint:\n"
        "\tpush rbp\n"
        "\tmov rbp, rsp\n"
        "\tsub rsp, 16\n"
        "\tmov [rbp - 4], edi\n"
        "\tmov eax, [rbp - 4]\n"
        "\tmov esi, eax\n"
        "\tlea rdi, [rel format]\n"
        "\txor eax, eax\n"
        "\tcall printf\n"
        "\tnop\n"
        "\tleave\n"
        "\tret\n"
        "main:\n"
        "\tpush rbp\n"
        "\tmov rbp, rsp\n";
    const char *postamble = "\txor eax, eax\n"
        "\tpop rbp\n"
        "\tret\n";

    size_t pre_len  = strlen(preamble);
    size_t post_len = strlen(postamble);
    
    CGState state;
    state_init(&state, 4, reg_names);
    state_alloc_atleast(&state, pre_len);
    strcpy(state.code + state.code_len, preamble);
    state.code_len += pre_len;
    
    int reg = gen_ast(ast, &state);
    cgprintint(&state, reg);

    state_alloc_atleast(&state, post_len);
    strcpy(state.code + state.code_len, postamble);
    state.code_len += post_len;
    
    state_free(&state);

    *code = state.code;

    return (Error){0, NULL};
}

