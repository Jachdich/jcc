#include "../include/parser.h"
#include "../include/lexer.h"

#include <stdlib.h>
#include <stdio.h>

int ast_gen(AST * ast, LexTokenStream *s) {
    s->pos = s->start;
    LexToken *t;
    AST *currAst = ast;
    while ((t = lex_consume(s))->type != TOK_EOF) {
        currAst->children = malloc(sizeof(AST));
        currAst->children_n = 1;
        currAst->tok = t;
        currAst = currAst->children;
        currAst->children_n = 0;
    }
    return 0;
}

void ast_print_internal(AST *ast, int n) {
    const char *type = toktostr(ast->tok->type);
    for (int i = 0; i < n; i++) {
        printf("    ");
    }
    printf("%s\n", type);
    for (size_t i = 0; i < ast->children_n; i++) {
        ast_print_internal(ast->children + i, n + 1);
    }
}

void ast_print(AST *ast) {
    ast_print_internal(ast, 0);
}

void ast_free(AST *ast) {
    if (ast->children != NULL && ast->children_n > 0) {
        for (size_t i = 0; i < ast->children_n; i++) {
            ast_free(ast->children + i);
        }
        free(ast->children);
    }
    ast->children = NULL;
}
