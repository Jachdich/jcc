#include "../include/parser.h"
#include "../include/lexer.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

ASTType lextoast(LexTokenType ty) {
    switch (ty) {
        case TOK_ADD:       return AST_ADD;
        case TOK_SUB:       return AST_SUB;
        case TOK_MUL:       return AST_MUL;
        case TOK_DIV:       return AST_DIV;
        case TOK_KINT:      return AST_KINT;
        case TOK_KCHAR:     return AST_KCHAR;
        case TOK_KSTRUCT:   return AST_KSTRUCT;
        case TOK_KVOID:     return AST_KVOID;
        case TOK_KENUM:     return AST_KENUM;
        case TOK_KLONG:     return AST_KLONG;
        case TOK_KSHORT:    return AST_KSHORT;
        case TOK_KRETURN:   return AST_KRETURN;
        case TOK_KIF:       return AST_KIF;
        case TOK_KELSE:     return AST_KELSE;
        case TOK_KDO:       return AST_KDO;
        case TOK_KWHILE:    return AST_KWHILE;
        case TOK_KFOR:      return AST_KFOR;
        case TOK_KSWITCH:   return AST_KSWITCH;
        case TOK_KCASE:     return AST_KCASE;
        case TOK_KBREAK:    return AST_KBREAK;
        case TOK_KCONTINUE: return AST_KCONTINUE;
        case TOK_KTYPEDEF:  return AST_KTYPEDEF;
        default: return AST_INVALID;
    }
}

VarType asttovar(ASTType ty) {
    switch (ty) {
        case AST_KINT: return VAR_KINT;
        case AST_KCHAR: return VAR_KCHAR;
        case AST_KSTRUCT: return VAR_KSTRUCT;
        case AST_KVOID: return VAR_KVOID;
        case AST_KENUM: return VAR_KENUM;
        case AST_KLONG: return VAR_KLONG;
        case AST_KSHORT: return VAR_KSHORT;
        default:
            printf("what the fuck, wrong type '%s' passed to asttovar\n", asttypetostr(ty));
            exit(0);
    }
}

void sym_init(SymTable *t, SymTable *outer) {
    t->pos = 0;
    t->capacity = 64;
    t->outer = outer;
    t->symbols = malloc(sizeof(Symbol) * 64);
}

size_t sym_new(SymTable *t, char *name, VarType type) {
    if (t->pos >= t->capacity) {
        t->symbols = realloc(t->symbols, sizeof(Symbol) * (t->capacity *= 2));
    }
    Symbol s;
    s.s = name;
    s.ident = t->pos;
    s.ty = type;
    t->symbols[t->pos++] = s;
    return s.ident;
}

size_t sym_find(SymTable *t, char *name) {
    for (size_t i = 0; i < t->pos; i++) {
        if (strcmp(name, t->symbols[i].s) == 0) {
            return i;
        }
    }
    return -1;
}

void sym_free(SymTable *t) {
    free(t->symbols);
    if (t->outer != NULL) {
        sym_free(t->outer);
    }
}

AST *ast_construct(AST **children, size_t children_n, ASTType type, size_t i) {
    AST *ast = malloc(sizeof(AST));
    ast->children = children;
    ast->children_n = children_n;
    ast->type = type;
    ast->i = i;
    return ast;
}

AST *number(LexTokenStream *s, SymTable *scope) {
    LexTokenType type = lex_peek(s)->type;
    if (type == TOK_INT) {
        LexToken *tok = lex_consume(s);
        return ast_construct(NULL, 0, AST_INT_LIT, tok->i);
    } else if (type == TOK_IDENT) {
        LexToken *tok = lex_consume(s);
        size_t ident = sym_find(scope, tok->str);
        if (ident == (size_t)-1) {
            fprintf(stderr, "Error: variable '%s' not defined", tok->str);
            exit(0);
        }
    } else {
        fprintf(stderr, "Error: expected int, got %s\n", toktostr(type));
        exit(0);
    }
}
AST *expr(LexTokenStream *s, SymTable *scope);

AST *vardecl(LexTokenStream *s, SymTable *scope) {
    LexTokenType type = lex_consume(s)->type;
    switch (type) {
    case TOK_KINT:
    case TOK_KCHAR:
    case TOK_KSTRUCT:
    case TOK_KVOID:
    case TOK_KENUM:
    case TOK_KLONG:
    case TOK_KSHORT:
        LexToken *t = lex_consume(s);
        if (t->type == TOK_IDENT) {
            size_t ident = sym_new(scope, t->str, asttovar(lextoast(type)));
            AST *child1 = ast_construct(NULL, 0, AST_LVIDENT, ident);
            LexToken *t2 = lex_consume(s);
            if (t2->type != TOK_ASSIGN) {
                if (t2->type == TOK_SEMICOLON) {
                    return NULL;
                } else {
                    fprintf(stderr, "Error: expected assignment operator or semicolon, got %s\n", toktostr(t->type));
                    exit(0);
                }
            }
            AST *child2 = expr(s, scope);

            AST **children = malloc(sizeof(AST*) * 2);
            children[0] = child1;
            children[1] = child2;
            AST *out = ast_construct(children, 2, AST_ASSIGN, 0);
            LexToken *tok = lex_consume(s);
            if (tok->type != TOK_SEMICOLON) {
                printf("what, expected a fucking semicolon you moron, got whatever the fuck '%s' is instead\n", toktostr(tok->type));
                exit(0);
            }
            return out;
        } else {
            fprintf(stderr, "Error: expected identifier, got %s\n", toktostr(t->type));
            exit(0);
        }
        break;
    default:
        fprintf(stderr, "Error: expected type identifier, got %s\n", toktostr(type));
        exit(0);
    }
}

AST *varassign(LexTokenStream *s, SymTable *scope) {
    LexToken *t = lex_consume(s);
    if (t->type != TOK_IDENT) {
        fprintf(stderr, "Error: expected identifier, got %s\n", toktostr(t->type));
        exit(0);
    }

    if (lex_consume(s)->type != TOK_ASSIGN) {
        fprintf(stderr, "Error: expected assignment operator or semicolon, got %s\n", toktostr(t->type));
        exit(0);
    }

    AST **children = malloc(sizeof(AST*) * 2);
    size_t ident = sym_find(scope, t->str);
    if (ident == (size_t)-1) {
        fprintf(stderr, "Error: variable '%s' not defined", t->str);
        exit(0);
    }
    children[0] = ast_construct(NULL, 0, AST_LVIDENT, ident);
    children[1] = expr(s, scope);
    if (lex_consume(s)->type != TOK_SEMICOLON) {
        printf("what\n");
        exit(0);
    }
    return ast_construct(children, 2, AST_ASSIGN, 0);
}

AST *mulexpr(LexTokenStream *s, SymTable *scope) {
    AST *lhs = number(s, scope);
    LexTokenType type = lex_peek(s)->type;
    if (type == TOK_SEMICOLON) {
        return lhs;
    }
    
    while (type == TOK_MUL || type == TOK_DIV) {
        LexToken *oper = lex_consume(s);
        AST *rhs = number(s, scope);
        AST *tmp = malloc(sizeof(AST));
        tmp->children = malloc(sizeof(AST*) * 2);
        tmp->children_n = 2;
        tmp->type = lextoast(oper->type);
        tmp->children[0] = lhs;
        tmp->children[1] = rhs;
        lhs = tmp;
        if (lex_peek(s)->type == TOK_SEMICOLON) break;
    }
    
    return lhs;
    //} else {
    //    fprintf(stderr, "Error: expected * or /, got %s\n", toktostr(type));
    //    //return lhs;
    //}
}

AST *expr(LexTokenStream *s, SymTable *scope) {
    AST *lhs = mulexpr(s, scope);
    LexTokenType type = lex_peek(s)->type;
    if (type == TOK_SEMICOLON) {
        return lhs;
    } else if (type == TOK_ADD || type == TOK_SUB) {
        while (1) {
            LexToken *oper = lex_consume(s);
            AST *rhs = mulexpr(s, scope);
            AST *tmp = malloc(sizeof(AST));
            tmp->children = malloc(sizeof(AST*) * 2);
            tmp->children_n = 2;
            tmp->type = lextoast(oper->type);
            tmp->children[0] = lhs;
            tmp->children[1] = rhs;
            lhs = tmp;
            if (lex_peek(s)->type == TOK_SEMICOLON) break;
        }
        return lhs;
    } else {
        fprintf(stderr, "Error: expected + or -, got %s\n", toktostr(type));
        return NULL;
    }
}

struct ASTList {
    AST **smts;
    size_t cap;
    size_t pos;
};

typedef struct ASTList ASTList;

void ast_list_append(ASTList *l, AST *smt) {
    if (l->cap < l->pos) {
        l->smts = realloc(l->smts, l->cap * 2);
        l->cap *= 2;
    }
    l->smts[l->pos++] = smt;
}

void ast_list_init(ASTList *l) {
    l->smts = malloc(64);
    l->cap = 64;
    l->pos = 0;
}

ASTList ASTstatements(LexTokenStream *s, SymTable *scope) {
    ASTList smts;
    ast_list_init(&smts);
    while (1) {
        switch (lex_peek(s)->type) {
            case TOK_KINT:
            case TOK_KCHAR:
            case TOK_KSTRUCT:
            case TOK_KVOID:
            case TOK_KENUM:
            case TOK_KLONG:
            case TOK_KSHORT:
                AST *a = vardecl(s, scope);
                if (a != NULL) {
                    ast_list_append(&smts, a);
                }
                break;
            case TOK_IDENT:
                ast_list_append(&smts, varassign(s, scope));
            case TOK_INT:
                ast_list_append(&smts, expr(s, scope));
            case TOK_EOF:
                return smts;
            default:
                fprintf(stderr, "Syntax error, token %s\n", toktostr(lex_peek(s)->type));
                exit(0);
        }
    }
}

int ast_gen(AST *ast, LexTokenStream *s) {
    s->pos = s->start;
    SymTable scope;
    sym_init(&scope, NULL);
    ASTList exprast = ASTstatements(s, &scope);

    AST a;
    a.children = exprast.smts;
    a.children_n = exprast.pos;
    a.type = AST_PROG;
    a.i = 0;
    *ast = a;

    //TODO manage memory of scope

    return 0;
}

const char *asttypetostr(ASTType ty) {
    switch (ty) {
        case AST_INVALID:    return "AST_INVALID   ";
        case AST_INT_LIT:    return "AST_INT_LIT   ";
        case AST_STR_LIT:    return "AST_STR_LIT   ";
        case AST_IDENT:      return "AST_IDENT     ";
        case AST_LVIDENT:    return "AST_LVIDENT   ";
        case AST_ADD:        return "AST_ADD       ";
        case AST_SUB:        return "AST_SUB       ";
        case AST_MUL:        return "AST_MUL       ";
        case AST_DIV:        return "AST_DIV       ";
        case AST_ASSIGN:     return "AST_ASSIGN    ";
        case AST_COMPARE:    return "AST_COMPARE   ";
        case AST_GT:         return "AST_GT        ";
        case AST_LT:         return "AST_LT        ";
        case AST_GTE:        return "AST_GTE       ";
        case AST_LTE:        return "AST_LTE       ";
        case AST_ADD_ASSIGN: return "AST_ADD_ASSIGN";
        case AST_SUB_ASSIGN: return "AST_SUB_ASSIGN";
        case AST_MUL_ASSIGN: return "AST_MUL_ASSIGN";
        case AST_DIV_ASSIGN: return "AST_DIV_ASSIGN";
        case AST_AND:        return "AST_AND       ";
        case AST_ANDAND:     return "AST_ANDAND    ";
        case AST_OR:         return "AST_OR        ";
        case AST_OROR:       return "AST_OROR      ";
        case AST_KINT:       return "AST_KINT      ";
        case AST_KCHAR:      return "AST_KCHAR     ";
        case AST_KSTRUCT:    return "AST_KSTRUCT   ";
        case AST_KVOID:      return "AST_KVOID     ";
        case AST_KENUM:      return "AST_KENUM     ";
        case AST_KLONG:      return "AST_KLONG     ";
        case AST_KSHORT:     return "AST_KSHORT    ";
        case AST_KRETURN:    return "AST_KRETURN   ";
        case AST_KIF:        return "AST_KIF       ";
        case AST_KELSE:      return "AST_KELSE     ";
        case AST_KDO:        return "AST_KDO       ";
        case AST_KWHILE:     return "AST_KWHILE    ";
        case AST_KFOR:       return "AST_KFOR      ";
        case AST_KSWITCH:    return "AST_KSWITCH   ";
        case AST_KCASE:      return "AST_KCASE     ";
        case AST_KBREAK:     return "AST_KBREAK    ";
        case AST_KCONTINUE:  return "AST_KCONTINUE ";
        case AST_KTYPEDEF:   return "AST_KTYPEDEF  ";
        case AST_DECL:       return "AST_DECL      ";
        case AST_DECL_VAL:   return "AST_DECL_VAL  ";
        case AST_PROG:       return "AST_PROG      ";
    }
    return "(AST TOKEN NOT RECOGNISED)";
}

void ast_print_internal(AST *ast, int n) {
    const char *type = asttypetostr(ast->type);
    for (int i = 0; i < n; i++) {
        printf("    ");
    }
    printf("%s %lu\n", type, ast->i);
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
