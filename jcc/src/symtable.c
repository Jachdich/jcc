#include "../include/symtable.h"
#include "../include/parser.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

size_t glob_sym_counter = 0;

void sym_init(SymTable *t, SymTable *outer) {
    t->pos = 0;
    t->capacity = 64;
    t->outer = outer;
    t->symbols = malloc(sizeof(Symbol) * 64);
    t->curr_stack_offset = 0;
    t->base_func_table = 0;
    t->label_n = 0;
}

Symbol *sym_new(SymTable *t, char *name, VarType type, int stype, int init_val) {
    if (t->pos >= t->capacity) {
        t->symbols = realloc(t->symbols, sizeof(Symbol) * (t->capacity *= 2));
    }
    Symbol s;
    s.s = name;
    s.ident = glob_sym_counter++;
    s.ty = type;
    s.stype = stype;
    s.init_value = init_val;
    s.is_stack = 0;
    t->symbols[t->pos] = s;
    return t->symbols + t->pos++;
}

Symbol *sym_func_new(SymTable *t, char *name, VarType type, struct FuncSig sig, int is_defined) {
    if (t->pos >= t->capacity) {
        t->symbols = realloc(t->symbols, sizeof(Symbol) * (t->capacity *= 2));
    }
    Symbol s;
    s.s = name;
    s.ident = glob_sym_counter++;
    s.ty = type;
    s.stype = S_FUNC;
    s.init_value = is_defined;
    s.is_stack = 0;
    s.sig = sig;
    t->symbols[t->pos] = s;
    return t->symbols + t->pos++;
}

Symbol *sym_stack_new(SymTable *t, char *name, VarType type) {
    if (t->pos >= t->capacity) {
        t->symbols = realloc(t->symbols, sizeof(Symbol) * (t->capacity *= 2));
    }
    Symbol s;
    s.s = name;
    s.ident = glob_sym_counter++;
    s.ty = type;
    s.stype = S_VAR;
    s.init_value = 0;
    s.is_stack = 1;

    SymTable *ft = t;
    while (!ft->base_func_table && ft->outer != NULL) {
        ft = ft->outer;
    }
    if (ft->outer == NULL) {
        fprintf(stderr, "Some bad error, tried to allocate stack variable but couldn't find the function in which to allocate it\n");
        exit(1);
    }
    s.stack_offset = ft->curr_stack_offset;
    ft->curr_stack_offset += varsize(type);
    
    t->symbols[t->pos] = s;
    return t->symbols + t->pos++;
}

char *sym_find(SymTable *table, size_t ident) {
    for (size_t i = 0; i < table->pos; i++) {
        if (table->symbols[i].ident == ident) {
            return table->symbols[i].s;
        }
    }
    if (table->outer != NULL) {
        return sym_find(table->outer, ident);
    }
    return NULL;
}


Symbol *sym_find_from_str(SymTable *t, char *name) {
    for (size_t i = 0; i < t->pos; i++) {
        if (strcmp(name, t->symbols[i].s) == 0) {
            return t->symbols + i;
        }
    }
    if (t->outer != NULL) {
        return sym_find_from_str(t->outer, name);
    }
    return NULL;
}

Symbol *sym_get(SymTable *t, char *name) {
    for (size_t i = 0; i < t->pos; i++) {
        if (strcmp(name, t->symbols[i].s) == 0) {
            return t->symbols + i;
        }
    }
    return NULL;
}

void sym_free(SymTable *t) {
    free(t->symbols);
}
