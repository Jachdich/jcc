#ifndef __PARSER_H
#define __PARSER_H
#include "../include/lexer.h"

struct AST;
typedef struct AST AST;

enum ASTType {
    AST_ADD,
    AST_SUB,
    AST_MUL,
    AST_DIV,
    AST_INTLIT,
    AST_STRLIT,
    AST_ASSIGN,
    AST_IDENT,
    AST_KINT,
};

typedef enum ASTType ASTType;

struct AST {
    AST **children;
    size_t children_n;
    ASTType type;
    size_t i;
    char *str;
};

int ast_gen(AST * ast, LexTokenStream *s);
void ast_print(AST *ast);
void ast_free(AST *ast);
#endif
