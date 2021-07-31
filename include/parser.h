#ifndef __PARSER_H
#define __PARSER_H
#include "../include/lexer.h"

struct AST;
typedef struct AST AST;

struct AST {
    AST *parent;
    AST *children;
    size_t children_n;
    LexToken *tok;
};

int ast_gen(AST * ast, LexTokenStream *s);
void ast_print(AST *ast);
void ast_free(AST *ast);
#endif
