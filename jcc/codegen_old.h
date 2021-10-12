#ifndef __CODEGEN_H
#define __CODEGEN_H
#include "../include/parser.h"
#include "../include/symtable.h"

Error cg_gen(AST *ast, char **code, SymTable *table, int debug);

#endif