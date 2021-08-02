#ifndef __CODEGEN_H
#define __CODEGEN_H
#include "../include/parser.h"

Error cg_gen(AST *ast, char **code);

#endif
