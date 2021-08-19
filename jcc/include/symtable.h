#ifndef __SYMTABLE_H
#define __SYMTABLE_H
#include "../include/parser.h"

typedef struct Symbol Symbol;
typedef struct SymTable SymTable;

struct Symbol {
    char *s;
    VarType ty;
    size_t ident;
    int stype;
    int init_value;
    int is_stack;
    int stack_offset;
};

struct SymTable {
    SymTable *outer;
    Symbol *symbols;
    size_t capacity;
    size_t pos;
    size_t curr_stack_offset;
    int base_func_table;
};

void sym_init(SymTable *t, SymTable *outer);
Symbol *sym_new(SymTable *t, char *name, VarType type, int stype, int init_val);
Symbol *sym_find_from_str(SymTable *t, char *name);
void sym_free(SymTable *t);
Symbol *sym_stack_new(SymTable *t, char *name, VarType type);
char *sym_find(SymTable *table, size_t ident);

#endif