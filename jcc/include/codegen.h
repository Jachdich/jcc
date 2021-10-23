#ifndef __CODEGEN_H
#define __CODEGEN_H
#include "../include/parser.h"
#include "../include/error.h"

Error cg_gen(AST *ast, char **code, int debug);

#endif
