#include "../include/parser.h"
#include "../include/lexer.h"

#include <stdlib.h>
#include <stdio.h>

AST *ast_construct(AST **children, size_t children_n, LexToken *tok) {
    AST *ast = malloc(sizeof(AST));
    ast->parent = NULL;
    ast->children = children;
    ast->children_n = children_n;
    ast->tok = tok;
    return ast;
}

AST *number(LexTokenStream *s) {
    LexTokenType type = lex_peek(s)->type;
    if (type == TOK_INT) {
        return ast_construct(NULL, 0, lex_consume(s));
    } else {
        fprintf(stderr, "Error: expected int, got %s\n", toktostr(type));
        exit(0);
    }
}

AST *mulexpr(LexTokenStream *s) {
    AST *lhs = number(s);
    LexTokenType type = lex_peek(s)->type;
    if (type == TOK_EOF) {
        return lhs;
    }
    
    while (type == TOK_MUL || type == TOK_DIV) {
        LexToken *oper = lex_consume(s);
        AST *rhs = number(s);
        AST *tmp = malloc(sizeof(AST));
        tmp->children = malloc(sizeof(AST*) * 2);
        tmp->children_n = 2;
        tmp->tok = oper;
        tmp->children[0] = lhs;
        tmp->children[1] = rhs;
        lhs = tmp;
        if (lex_peek(s)->type == TOK_EOF) break;
    }
    
    return lhs;
    //} else {
    //    fprintf(stderr, "Error: expected * or /, got %s\n", toktostr(type));
    //    //return lhs;
    //}
}

AST *expr(LexTokenStream *s) {
    AST *lhs = mulexpr(s);
    LexTokenType type = lex_peek(s)->type;
    if (type == TOK_EOF) {
        return lhs;
    } else if (type == TOK_ADD || type == TOK_SUB) {
        while (1) {
            LexToken *oper = lex_consume(s);
            AST *rhs = mulexpr(s);
            AST *tmp = malloc(sizeof(AST));
            tmp->children = malloc(sizeof(AST*) * 2);
            tmp->children_n = 2;
            tmp->tok = oper;
            tmp->children[0] = lhs;
            tmp->children[1] = rhs;
            lhs = tmp;
            printf("TYPE IN EXPR: %d %s\n", oper->type, toktostr(oper->type));
            if (lex_peek(s)->type == TOK_EOF) break;
        }
        return lhs;
    } else {
        fprintf(stderr, "Error: expected + or -, got %s\n", toktostr(type));
    }
}

int ast_gen(AST *ast, LexTokenStream *s) {
    s->pos = s->start;
    //LexToken *t;
    AST *exprast = expr(s);
    *ast = *exprast;
    free(exprast);
    //AST *currAst = ast;
    /*
    while ((t = lex_consume(s))->type != TOK_EOF) {
        currAst->children = malloc(sizeof(AST));
        currAst->children_n = 1;
        currAst->tok = t;
        currAst = currAst->children;
        currAst->children_n = 0;
    }*/
    return 0;
}

void ast_print_internal(AST *ast, int n) {
    const char *type = toktostr(ast->tok->type);
    for (int i = 0; i < n; i++) {
        printf("    ");
    }
    printf("%s\n", type);
    for (size_t i = 0; i < ast->children_n; i++) {
        ast_print_internal(ast->children[i], n + 1);
    }
}

void ast_print(AST *ast) {
    ast_print_internal(ast, 0);
}

void ast_free(AST *ast) {
    if (ast->children != NULL && ast->children_n > 0) {
        for (size_t i = 0; i < ast->children_n; i++) {
            ast_free(ast->children[i]);
        }
        free(ast->children);
    }
    ast->children = NULL;
}
