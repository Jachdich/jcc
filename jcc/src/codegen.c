#include "../include/codegen.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//max length of a register's string representation
#define REGSTRLEN 3

//maximum length a 64 bit int can be when converted to str
#define MAXINTSTRLEN 20

struct CGState {
    char *c;
    size_t cl;
    size_t code_capacity;
    size_t num_regs;
    char *regs_free;
    char **reg_names;
    char **op_names;
    SymTable *table;
    int label_ident;
    int debug;
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
    CG_GTE,
    CG_MOD,
};

typedef struct CGState CGState;

void state_alloc_atleast(CGState *s, size_t bytes) {
    //sanity check
    if (bytes <= 0) {
        fprintf(stderr, "WARNING: state_alloc_atleast() was called with bytes <= 0. This is almost certainly a bug\n");
    }
    //if already enough bytes
    if (s->code_capacity - s->cl > bytes) {
        return;
    }
    //allocate double the memory until we have enough
    size_t to_alloc = s->code_capacity * 2;
    while (to_alloc - s->cl < bytes) {
        to_alloc *= 2;
    }
    s->c = realloc(s->c, to_alloc);
    s->code_capacity = to_alloc;
}

char *state_gen_label(CGState *s) {
    char *res = malloc(8);
    sprintf(res, "L%03d", s->label_ident++);
    return res;
}

void state_init(CGState *s, size_t num_regs, char **reg_names, char **op_names, struct SymTable *table, int debug) {
    s->regs_free = malloc(num_regs);
    s->num_regs = num_regs;
    s->reg_names = reg_names;
    for (size_t i = 0; i < num_regs; i++) {
        s->regs_free[i] = 0;
    }
    s->c = malloc(8);
    s->cl = 0;
    s->code_capacity = 8;
    s->table = table;
    s->op_names = op_names;
    s->label_ident = 0;
    s->debug = debug;
}

void state_free(CGState *s) {
    free(s->regs_free);
}

int reg_alloc(CGState *state) {
    for (size_t i = 0; i < state->num_regs; i++) {
        if (state->regs_free[i] == 0) {
            state->regs_free[i] = 1;
            return i;
        }
    }
    printf("Internal error: Out of registers\n");
    return -1;
}

void reg_free(CGState *state, int reg) {
    state->regs_free[reg] = 0;
}

int cgmathop(int rega, int regb, int op, CGState *s) {
    state_alloc_atleast(s, 12 + REGSTRLEN * 3);
    s->cl += sprintf(s->c + s->cl, "\t%s\t%s, %s -> %s\n", s->op_names[op], s->reg_names[rega], s->reg_names[regb], s->reg_names[rega]);
    reg_free(s, regb);
    return rega;
}

int cgloadint(int val, CGState *s) {
    int reg = reg_alloc(s);

    char op = 'l';
    if (val < 32768) {
        //the value is small enough to
        //save 4 bytes by putting it into the arguments
        //instead of a new word
        op = 'i';
    }
    
    state_alloc_atleast(s, 11 + REGSTRLEN + MAXINTSTRLEN);
    s->cl += sprintf(s->c + s->cl, "\tmov%c\t%d -> %s\n", op, val, s->reg_names[reg]);
    return reg;
}

int cgprintint(CGState *s, int reg) {
    state_alloc_atleast(s, 6 + REGSTRLEN);
    s->cl += sprintf(s->c + s->cl, "\tout\t%s\n", s->reg_names[reg]);
    reg_free(s, reg);
    return -1;
}
int cgstoreglob(CGState *s, int reg, char *ident) {
    state_alloc_atleast(s, 12 + REGSTRLEN + strlen(ident));
    s->cl += sprintf(s->c + s->cl, "\tmovra\t%s -> %s\n", s->reg_names[reg], ident);
    reg_free(s, reg);
    return -1;
}

int cgloadglob(CGState *s, char *ident) {
    int reg = reg_alloc(s);
    state_alloc_atleast(s, 12 + REGSTRLEN + strlen(ident));
    s->cl += sprintf(s->c + s->cl, "\tmovar\t%s -> %s\n", ident, s->reg_names[reg]);
    return reg;
}

int cgloadlocal(CGState *s, char *ident) {
    int reg = reg_alloc(s);
    //int temp = reg_alloc(s);
    int offset = sym_find_from_str(s->table, ident)->stack_offset;

    if (s->debug) {
        state_alloc_atleast(s, 36 + REGSTRLEN + strlen(ident));
        s->cl += sprintf(s->c + s->cl, "\n\t; Loading local variable '%s' into %s\n", ident, s->reg_names[reg]);
    }

    /*
    state_alloc_atleast(s, 34 + REGSTRLEN * 4 + MAXINTSTRLEN);
    s->cl += sprintf(s->c + s->cl, "\tmov\trbp -> %s\n",  s->reg_names[temp]);
    s->cl += sprintf(s->c + s->cl, "\taddi\t%s, %d\n",    s->reg_names[temp], offset);
    s->cl += sprintf(s->c + s->cl, "\tdrefr\t%s -> %s\n", s->reg_names[temp], s->reg_names[reg]);*/
    state_alloc_atleast(s, 12 + MAXINTSTRLEN + REGSTRLEN);
    s->cl += sprintf(s->c + s->cl, "\tmovbp\t%d -> %s\n", offset, s->reg_names[reg]);
    //reg_free(s, temp);
    return reg;
}

int cgstorelocal(CGState *s, int reg, char *ident) {
    //int temp = reg_alloc(s);
    int offset = sym_find_from_str(s->table, ident)->stack_offset;

    if (s->debug) {
        state_alloc_atleast(s, 36 + REGSTRLEN + strlen(ident));
        s->cl += sprintf(s->c + s->cl, "\n\t; Storing %s into local variable '%s'\n", s->reg_names[reg], ident);
    }
    //state_alloc_atleast(s, 34 + REGSTRLEN * 4 + MAXINTSTRLEN);
    //s->cl += sprintf(s->c + s->cl, "\tmov\trbp -> %s\n",  s->reg_names[temp]);
    //s->cl += sprintf(s->c + s->cl, "\taddi\t%s, %d\n",    s->reg_names[temp], offset);
    //s->cl += sprintf(s->c + s->cl, "\tdrefw\t%s -> %s\n", s->reg_names[reg], s->reg_names[temp]);
    //reg_free(s, temp);
    state_alloc_atleast(s, 12 + MAXINTSTRLEN + REGSTRLEN);
    s->cl += sprintf(s->c + s->cl, "\tmovpb\t%s -> %d\n", s->reg_names[reg], offset);
    return -1;
}

int cgloadvar(CGState *s, char *ident) {
    Symbol *sym = sym_find_from_str(s->table, ident);
    if (sym->is_stack) {
        return cgloadlocal(s, ident);
    } else {
        return cgloadglob(s, ident);
    }
}

int cgstorevar(CGState *s, int reg, char *ident) {
    Symbol *sym = sym_find_from_str(s->table, ident);
    if (sym->is_stack) {
        return cgstorelocal(s, reg, ident);
    } else {
        return cgstoreglob(s, reg, ident);
    }
}

int gen_ast(AST *ast, CGState *state, int reg);

int cgif(CGState *s, AST *ast) {
    if (s->debug) {
        state_alloc_atleast(s, 18);
        s->cl += sprintf(s->c + s->cl, "\n\t; If: condition\n");
    }
    int cond = gen_ast(ast->children[0], s, -1);
    char *false_label = NULL;
    if (ast->children_n == 3) {
        false_label = state_gen_label(s);
    }
    char *end_label = state_gen_label(s);

    char *l;
    if (ast->children_n == 3) {
        l = false_label;
    } else {
        l = end_label;
    }

    if (s->debug) {
        state_alloc_atleast(s, 24);
        s->cl += sprintf(s->c + s->cl, "\n\t; jump to false label\n");
    }
    state_alloc_atleast(s, 8 + strlen(l) + REGSTRLEN);
    s->cl += sprintf(s->c + s->cl, "\tjz\t%s, %s\n", l, s->reg_names[cond]);
    if (s->debug) {
        state_alloc_atleast(s, 24);
        s->cl += sprintf(s->c + s->cl, "\n\t; If: true case\n");
    }
    gen_ast(ast->children[1], s, -1);

    if (ast->children_n == 3) {
        if (s->debug) {
            state_alloc_atleast(s, 24);
            s->cl += sprintf(s->c + s->cl, "\n\t; If: false/else case\n");
        }
        state_alloc_atleast(s, 7 + strlen(false_label) + strlen(end_label));
        s->cl += sprintf(s->c + s->cl, "\tjp %s\n%s:\n", end_label, false_label);
        gen_ast(ast->children[2], s, -1);
    }
    
    state_alloc_atleast(s, 2 + strlen(end_label));
    s->cl += sprintf(s->c + s->cl, "%s:\n", end_label);
    free(end_label);
    free(false_label);

    return -1;
}

int cgwhile(CGState *s, AST *ast) {
    char *start_label = state_gen_label(s);
    char *end_label = state_gen_label(s);
    state_alloc_atleast(s, 2 + strlen(start_label));
    s->cl += sprintf(s->c + s->cl, "%s:\n", start_label);
    int reg = gen_ast(ast->children[0], s, -1);

    state_alloc_atleast(s, 8 + REGSTRLEN + strlen(end_label));
    s->cl += sprintf(s->c + s->cl, "\tjz\t%s, %s\n", end_label, s->reg_names[reg]);

    gen_ast(ast->children[1], s, -1);

    state_alloc_atleast(s, 2 + 5 + strlen(start_label) + strlen(end_label));
    s->cl += sprintf(s->c + s->cl, "\tjp\t%s\n%s:\n", start_label, end_label);

    return -1;
}

int cgfunc(CGState *s, AST *ast) {
    size_t ident = ast->i;
    char *label = sym_find(s->table, ident);
    int num_vars = ast->children[0]->scope->curr_stack_offset;
    state_alloc_atleast(s, 40 + strlen(label) + MAXINTSTRLEN);
    s->cl += sprintf(s->c + s->cl, "%s:\n\tpush\trbp\n\tmov\trsp -> rbp\n\taddi\trsp, %d\n", label, num_vars);

    /*
    int t1_r = reg_alloc(s);
    //int t2_r = reg_alloc(s);
    const char *t1 = s->reg_names[t1_r];
    //const char *t2 = s->reg_names[t2_r];
    for (size_t idx = 0; idx < ast->children[0]->children_n; idx++) {
        AST *arg = ast->children[0]->children[idx];
        int offset = sym_find_from_str(s->table, sym_find(s->table, arg->i))->stack_offset;
        if (s->debug) {
            state_alloc_atleast(s, 52 + MAXINTSTRLEN + strlen(sym_find(s->table, arg->i)));
            s->cl += sprintf(s->c + s->cl, "\t; Loading argument '%s' (orig value at rbp offset -%d)\n", sym_find(s->table, arg->i), 4 * (int)(idx + 2));
        }
        //state_alloc_atleast(s, 68 + strlen(t1) + strlen(t2) + MAXINTSTRLEN * 2);
        //s->cl += sprintf(s->c + s->cl, "\tmov\trbp -> %s\n",  t1);
        //s->cl += sprintf(s->c + s->cl, "\tsubi\t%s, %d\n",    t1, 4 * ((int)idx + 2));
        //s->cl += sprintf(s->c + s->cl, "\tdrefr\t%s -> %s\n", t1, t2);
        //s->cl += sprintf(s->c + s->cl, "\tmov\trbp -> %s\n",  t1);
        //s->cl += sprintf(s->c + s->cl, "\taddi\t%s, %d\n",    t1, offset);
        //s->cl += sprintf(s->c + s->cl, "\tdrefw\t%s -> %s\n", t2, t1);
        state_alloc_atleast(s, 24 + strlen(t1) * 2 + MAXINTSTRLEN * 2);
        s->cl += sprintf(s->c + s->cl, "\tmovbp\t%d -> %s\n", -4 * ((int)idx + 2), t1);
        s->cl += sprintf(s->c + s->cl, "\tmovpb\t%s -> %d\n", t1, offset);
    }
    reg_free(s, t1_r);
    //reg_free(s, t2_r);*/
    
    gen_ast(ast->children[1], s, -1);

    state_alloc_atleast(s, 26 + MAXINTSTRLEN);
    s->cl += sprintf(s->c + s->cl, "\tsubi\trsp, %d\n\tpop\trbp\n\tret\n", num_vars);
    return -1;
}

int cgfunccall(CGState *s, int ident, size_t n_args) {
    char *label = sym_find(s->table, ident);
    Symbol *func= sym_find_from_str(s->table, label);
    state_alloc_atleast(s, 7 + 12 + strlen(label));
    s->cl += sprintf(s->c + s->cl, "\tcall\t%s\n", label);
    int reg = -1;
    if (func->ty != VAR_VOID) {
        reg = reg_alloc(s);
        state_alloc_atleast(s, 7 + 12 + 6 + strlen(label));
        s->cl += sprintf(s->c + s->cl, "\tpop\t%s\n", s->reg_names[reg]);
    }
    s->cl += sprintf(s->c + s->cl, "\tsubi\trsp, %d\n", (int)(n_args * 4));
    
    return reg;
}

int cgsetupargs(CGState *s, int *regs, size_t num) {
    for (int i = num - 1; i >= 0; i--) {
        state_alloc_atleast(s, 7 + REGSTRLEN);
        s->cl += sprintf(s->c + s->cl, "\tpush\t%s\n", s->reg_names[regs[i]]);
    }
    return -1;
}

int cgret(CGState *s, int reg) {
    state_alloc_atleast(s, 12 + REGSTRLEN);
    s->cl += sprintf(s->c + s->cl, "\tpush\t%s\n\tret\n", s->reg_names[reg]);
}

int gen_ast(AST *ast, CGState *state, int reg) {
    SymTable *tab = ast->scope;
    SymTable *orig = state->table;    if (tab != NULL) {
        state->table = tab;
    }
    
    switch(ast->type) {
        case AST_KIF:
            return cgif(state, ast);
        case AST_KWHILE:
            return cgwhile(state, ast);
        case AST_FUNC:
            return cgfunc(state, ast);
        default: break;
    }

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
        case AST_MODULO: res = cgmathop(ch_regs[0], ch_regs[1], CG_MOD, state); break;
        case AST_COMPARE:res = cgmathop(ch_regs[0], ch_regs[1], CG_CMP, state); break;
        case AST_GT:     res = cgmathop(ch_regs[0], ch_regs[1], CG_GT, state); break;
        case AST_GTE:    res = cgmathop(ch_regs[0], ch_regs[1], CG_GTE, state); break;
        case AST_LT:     res = cgmathop(ch_regs[0], ch_regs[1], CG_LT, state); break;
        case AST_LTE:    res = cgmathop(ch_regs[0], ch_regs[1], CG_LTE, state); break;
        case AST_INT_LIT:res = cgloadint(ast->i, state); break;
        case AST_IDENT:  res = cgloadvar(state, sym_find(state->table, ast->i)); break;
        case AST_LVIDENT:cgstorevar(state, reg, sym_find(state->table, ast->i)); break;
        case AST_ASSIGN: res = ch_regs[0]; break;
        case AST_KPRINT: cgprintint(state, ch_regs[0]); break;
        case AST_FUNCCALL: res = cgfunccall(state, ast->i, ast->children[0]->children_n); break;
        case AST_ARGLIST:  res = cgsetupargs(state, ch_regs, ast->children_n); break;
        case AST_KRETURN: res = cgret(state, ch_regs[0]); break;
        default:
            fprintf(stderr, "Error: unrecognised token '%s'\n", asttypetostr(ast->type));
            exit(1);
            break;
    }

    if (ast->children_n > 0) {
        free(ch_regs);
    }
    state->table = orig;
    return res;
}

Error cg_gen(AST *ast, char **code, SymTable *table, int debug) {
    char *reg_names[] = {"r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"};
    char *op_names[]  = {"add", "sub", "mul", "div", "cmp", "lt", "lte", "gt", "gte", "mod"};

    CGState state;
    state_init(&state, 16, reg_names, op_names, table, debug);
    
    gen_ast(ast, &state, -1);

    state_free(&state);

    *code = state.c;

    return (Error){0, NULL};
}

