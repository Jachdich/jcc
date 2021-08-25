#include "../include/parser.h"
#include "../include/lexer.h"
#include "../include/symtable.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

ASTType lextoast(LexTokenType ty) {
    switch (ty) {
        case TOK_ADD:       return AST_ADD;
        case TOK_SUB:       return AST_SUB;
        case TOK_MUL:       return AST_MUL;
        case TOK_DIV:       return AST_DIV;
        case TOK_MODULO:    return AST_MODULO;

        case TOK_GT:        return AST_GT;
        case TOK_GTE:       return AST_GTE;
        case TOK_LT:        return AST_LT;
        case TOK_LTE:       return AST_LTE;
        case TOK_COMPARE:   return AST_COMPARE;
        
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
        case AST_KINT:    return VAR_INT;
        case AST_KCHAR:   return VAR_CHAR;
        case AST_KSTRUCT: return VAR_STRUCT;
        case AST_KVOID:   return VAR_VOID;
        case AST_KENUM:   return VAR_ENUM;
        case AST_KLONG:   return VAR_LONG;
        case AST_KSHORT:  return VAR_SHORT;
        default:
            printf("what the fuck, wrong type '%s' passed to asttovar\n", asttypetostr(ty));
            exit(1);
    }
}

int varsize(VarType ty) {
    switch (ty) {
        case VAR_INT: return 4;
        case VAR_CHAR: return 1;
        case VAR_STRUCT: return -1;
        case VAR_VOID: return 0;
        case VAR_ENUM: return 4;
        case VAR_LONG: return 4;
        case VAR_SHORT: return 2;
        case VAR_NONE: return 0;
    }
}

AST *ast_construct(AST **children, size_t children_n, ASTType type, size_t i, VarType ty) {
    AST *ast = malloc(sizeof(AST));
    ast->children = children;
    ast->children_n = children_n;
    ast->type = type;
    ast->i = i;
    ast->vartype = ty;
    ast->scope = NULL;
    return ast;
}

AST *ast_construct_with_scope(AST **children, size_t children_n, ASTType type, size_t i, VarType ty, SymTable *t) {
    AST *ast = ast_construct(children, children_n, type, i, ty);
    ast->scope = t;
    return ast;
}

VarType min_int_size(size_t i) {
    if (i < 256) {
        return VAR_CHAR;
    } else if (i < 65536) {
        return VAR_SHORT;
    } else if (i < 4294967296) {
        return VAR_INT;
    } else {
        return VAR_LONG;
    }
}


VarType combine_types(VarType a, VarType b) {
    if (a == VAR_VOID || b == VAR_VOID) {
        fprintf(stderr, "Error: void type expression used in expression\n");
        //exit(1);
        //cause a segfault so I can get a trace on what called this func
        char *a = 0x0;
        *a = 1;
    }

    //fuck off I'll fix it later
    return VAR_INT;
}
AST *funccall(LexTokenStream *s, SymTable *scope);
AST *number(LexTokenStream *s, SymTable *scope) {
    LexTokenType type = lex_peek(s)->type;
    if (type == TOK_INT) {
        LexToken *tok = lex_consume(s);
        VarType ty = min_int_size(tok->i);
        
        return ast_construct(NULL, 0, AST_INT_LIT, tok->i, ty);
    } else if (type == TOK_IDENT) {
        if (lex_peek_n(s, 2)->type == TOK_OPAREN) {
            return funccall(s, scope);
        }
        LexToken *tok = lex_consume(s);
        Symbol *s = sym_find_from_str(scope, tok->str);
        if (s == NULL) {
            fprintf(stderr, "Ln %d: Error: variable '%s' not defined\n", tok->linenum, tok->str);
            exit(1);
        }
        return ast_construct(NULL, 0, AST_IDENT, s->ident, s->ty);
    } else {
        fprintf(stderr, "Error: expected int, got %s\n", toktostr(type));
        exit(1);
    }
}
AST *expr(LexTokenStream *s, SymTable *scope);

AST *localvardecl(LexTokenStream *s, SymTable *scope) {
    LexToken *tok = lex_consume(s);
    switch (tok->type) {
    case TOK_KINT:
    case TOK_KCHAR:
    case TOK_KSTRUCT:
    case TOK_KVOID:
    case TOK_KENUM:
    case TOK_KLONG:
    case TOK_KSHORT: {
        LexToken *t = lex_consume_assert(s, TOK_IDENT);
        if (sym_get(scope, t->str) != NULL) {
            fprintf(stderr, "Ln %d: Error: local variable '%s' redefined\n", t->linenum, t->str);
            exit(1);
        }
        Symbol *sym = sym_stack_new(scope, t->str, asttovar(lextoast(tok->type)));
        LexToken *t2 = lex_consume(s);
        if (t2->type != TOK_ASSIGN) {
            if (t2->type == TOK_SEMICOLON) {
                return NULL;
            } else {
                fprintf(stderr, "Ln %d: Error: expected assignment operator or semicolon, got %s\n", t->linenum, toktostr(t->type));
                exit(1);
            }
        }
        AST *child1 = ast_construct(NULL, 0, AST_LVIDENT, sym->ident, VAR_NONE);
        AST *child2 = expr(s, scope);

        AST **children = malloc(sizeof(AST*) * 2);
        children[1] = child1;
        children[0] = child2;
        AST *out = ast_construct(children, 2, AST_ASSIGN, 0, VAR_NONE);
        return out;

        break;
    }
    default:
        fprintf(stderr, "Ln %d: Error: expected type identifier, got %s\n", tok->linenum, toktostr(tok->type));
        exit(1);
    }
}

AST *globvardef(LexTokenStream *s, SymTable *scope) {
    LexToken *tok = lex_consume(s);
    switch (tok->type) {
    case TOK_KINT:
    case TOK_KCHAR:
    case TOK_KSTRUCT:
    case TOK_KVOID:
    case TOK_KENUM:
    case TOK_KLONG:
    case TOK_KSHORT: {
        LexToken *t = lex_consume_assert(s, TOK_IDENT);
        if (sym_get(scope, t->str) != NULL) {
            fprintf(stderr, "Ln %d: Error: global variable '%s' redefined\n", t->linenum, t->str);
            exit(1);
        }
        LexToken *t2 = lex_peek(s);
        if (t2->type != TOK_ASSIGN) {
            if (t2->type == TOK_SEMICOLON) {
                sym_new(scope, t->str, asttovar(lextoast(tok->type)), S_VAR, 0);
                return NULL;
            } else {
                fprintf(stderr, "Ln %d: Error: expected assignment operator or semicolon, got %s\n", t->linenum, toktostr(t->type));
                exit(1);
            }
        }
        lex_consume_assert(s, TOK_ASSIGN);
        AST *child = number(s, scope);
        if (child->type != AST_INT_LIT) {
            fprintf(stderr, "Ln %d: Error: only integer initialisers for global variables are currently supported\n", t2->linenum);
            exit(1);
        }
        sym_new(scope, t->str, asttovar(lextoast(tok->type)), S_VAR, child->i);
        free(child);
        return NULL;
    }
    default:
        fprintf(stderr, "Ln %d: Error: expected type identifier, got %s\n", tok->linenum, toktostr(tok->type));
        exit(1);
    }
}

AST *varassign(LexTokenStream *s, SymTable *scope) {
    LexToken *t = lex_consume_assert(s, TOK_IDENT);
    lex_consume_assert(s, TOK_ASSIGN);

    AST **children = malloc(sizeof(AST*) * 2);
    Symbol *sym = sym_find_from_str(scope, t->str);
    if (sym == NULL) {
        fprintf(stderr, "Ln %d: Error: variable '%s' not defined\n", t->linenum, t->str);
        exit(1);
    }
    children[1] = ast_construct(NULL, 0, AST_LVIDENT, sym->ident, sym->ty);
    children[0] = expr(s, scope);
    if (combine_types(sym->ty, children[0]->vartype) != sym->ty) {
        //one day, this will annoy me enough to fix it
        //for now, stfu
        fprintf(stderr, "Ln %d: Error: Narrowing or invalid conversion from %s to %s\n", t->linenum, (char*)NULL, (char*)NULL);
        exit(1);
    }
    return ast_construct(children, 2, AST_ASSIGN, 0, VAR_NONE);
}


AST *printsmt(LexTokenStream *s, SymTable *scope) {
    lex_consume_assert(s, TOK_KPRINT);
    AST *e = expr(s, scope);
    AST **child = malloc(sizeof(AST*));
    child[0] = e;
    AST *ret = ast_construct(child, 1, AST_KPRINT, 0, VAR_NONE);
    return ret;
}

AST *mulexpr(LexTokenStream *s, SymTable *scope) {
    AST *lhs = number(s, scope);
    LexTokenType type = lex_peek(s)->type;
    while (type == TOK_MUL || type == TOK_DIV || type == TOK_MODULO) {
        LexToken *oper = lex_consume(s);
        AST *rhs = number(s, scope);
        AST **children = malloc(sizeof(AST*) * 2);
        children[0] = lhs;
        children[1] = rhs;
        lhs = ast_construct(children, 2, lextoast(oper->type), 0, combine_types(lhs->vartype, rhs->vartype));
        type = lex_peek(s)->type;
    }
    
    return lhs;

}

AST *addexpr(LexTokenStream *s, SymTable *scope) {
    AST *lhs = mulexpr(s, scope);
    LexTokenType type = lex_peek(s)->type;
    while (type == TOK_ADD || type == TOK_SUB) {
        LexToken *oper = lex_consume(s);
        AST *rhs = mulexpr(s, scope);
        AST **children = malloc(sizeof(AST*) * 2);
        children[0] = lhs;
        children[1] = rhs;
        lhs = ast_construct(children, 2, lextoast(oper->type), 0, combine_types(lhs->vartype, rhs->vartype));
        type = lex_peek(s)->type;
    }
    return lhs;
}

AST *expr(LexTokenStream *s, SymTable *scope) {
    AST *lhs = addexpr(s, scope);
    LexTokenType type = lex_peek(s)->type;
    while (type == TOK_COMPARE || type == TOK_GT || type == TOK_GTE || type == TOK_LT || type == TOK_LTE) {
        LexToken *oper = lex_consume(s);
        AST *rhs = addexpr(s, scope);
        AST **children = malloc(sizeof(AST*) * 2);
        children[0] = lhs;
        children[1] = rhs;
        lhs = ast_construct(children, 2, lextoast(oper->type), 0, combine_types(lhs->vartype, rhs->vartype));
        type = lex_peek(s)->type;
    }
    return lhs;
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
AST *statement(LexTokenStream *s, SymTable *scope);

AST *compoundsmt(LexTokenStream *s, SymTable *scope) {
    lex_consume_assert(s, TOK_OBRACE);
    LexToken *t;
    ASTList lst;
    ast_list_init(&lst);

    SymTable *tab = malloc(sizeof(*tab));
    sym_init(tab, scope);
    
    while ((t = lex_peek(s))->type != TOK_CBRACE) {
        AST *a = statement(s, tab);
        if (a != NULL) {
            ast_list_append(&lst, a);
            if (a->type == AST_KPRINT || a->type == AST_ASSIGN || a->type == AST_FUNCCALL) {
                lex_consume_assert(s, TOK_SEMICOLON);
            }
        }
    }
    lex_consume_assert(s, TOK_CBRACE);
    return ast_construct_with_scope(lst.smts, lst.pos, AST_PROG, 0, VAR_NONE, tab);
}

AST *ifsmt(LexTokenStream *s, SymTable *scope) {
    lex_consume_assert(s, TOK_KIF);
    lex_consume_assert(s, TOK_OPAREN);
    AST *a = expr(s, scope);
    lex_consume_assert(s, TOK_CPAREN);
    AST *b = compoundsmt(s, scope);
    AST *c = NULL;
    if (lex_peek(s)->type == TOK_KELSE) {
        lex_consume_assert(s, TOK_KELSE);
        if (lex_peek(s)->type == TOK_KIF) {
            c = ifsmt(s, scope);
        } else {
            c = compoundsmt(s, scope);
        }
    }
    int cn = 2;
    if (c != NULL)
        cn = 3;
    
    AST **children = malloc(sizeof(AST*) * cn);
    children[0] = a;
    children[1] = b;
    if (c != NULL)
        children[2] = c;
    return ast_construct(children, cn, AST_KIF, 0, VAR_NONE);
}

AST *whilesmt(LexTokenStream *s, SymTable *scope) {
    lex_consume_assert(s, TOK_KWHILE);
    lex_consume_assert(s, TOK_OPAREN);
    AST *a = expr(s, scope);
    lex_consume_assert(s, TOK_CPAREN);
    AST *b = compoundsmt(s, scope);
    AST **children = malloc(sizeof(AST*) * 2);
    children[0] = a;
    children[1] = b;
    return ast_construct(children, 2, AST_KWHILE, 0, VAR_NONE);
}

AST *forsmt(LexTokenStream *s, SymTable *scope) {
    lex_consume_assert(s, TOK_KFOR);
    lex_consume_assert(s, TOK_OPAREN);
    AST *init = statement(s, scope);
    lex_consume_assert(s, TOK_SEMICOLON);
    AST *cond = expr(s, scope);
    lex_consume_assert(s, TOK_SEMICOLON);
    AST *iter = statement(s, scope);
    lex_consume_assert(s, TOK_CPAREN);
    AST *body = compoundsmt(s, scope);

    AST **ch1 = malloc(sizeof(AST*) * 2);
    AST **ch2 = malloc(sizeof(AST*) * 2);
    AST **ch3 = malloc(sizeof(AST*) * 2);

    ch3[0] = body;
    ch3[1] = iter;

    ch2[0] = cond;
    ch2[1] = ast_construct(ch3, 2, AST_PROG, 0, VAR_NONE);

    ch1[0] = init;
    ch1[1] = ast_construct(ch2, 2, AST_KWHILE, 0, VAR_NONE);
    return ast_construct(ch1, 2, AST_PROG, 0, VAR_NONE);
}

int is_sig_compatible(struct FuncSig func, struct FuncSig caller) {
    if (func.numargs != caller.numargs) {
        return 0;
    }
    return 1;
}

AST *funccall(LexTokenStream *s, SymTable *scope) {
    LexToken *tok = lex_consume_assert(s, TOK_IDENT);
    Symbol *sym = sym_find_from_str(scope, tok->str);
    if (sym == NULL) {
        fprintf(stderr, "Ln %d: Error: Function '%s' is not defined\n", tok->linenum, tok->str);
        exit(1);
    };

    lex_consume_assert(s, TOK_OPAREN);
    ASTList lst;
    ast_list_init(&lst);
    struct FuncSig sig;
    sig.argtypes = malloc(sizeof(VarType) * 64);
    sig.numargs = 0;
    int cap = 64;
    while (lex_peek(s)->type != TOK_CPAREN) {
        AST *exprast = expr(s, scope);
        if (lex_peek(s)->type != TOK_CPAREN) {
            lex_consume_assert(s, TOK_COMMA);
        }
        sig.argtypes[sig.numargs++] = exprast->vartype;
        if (sig.numargs >= cap) {
            sig.argtypes = realloc(sig.argtypes, cap *= 2);
        }
        ast_list_append(&lst, exprast);
    }
    if (!is_sig_compatible(sym->sig, sig)) {
        fprintf(stderr, "Ln %d: Error: attempt to call function '%s' with incorrect arguments (TODO expand this message)\n", lex_peek_n(s, -1)->linenum, tok->str);
        exit(1);
    }
    lex_consume_assert(s, TOK_CPAREN);
    AST *arglist = ast_construct_with_scope(lst.smts, lst.pos, AST_ARGLIST, 0, VAR_NONE, scope);
    AST **ch = malloc(sizeof(*ch));
    ch[0] = arglist;
    return ast_construct(ch, 1, AST_FUNCCALL, sym->ident, sym->ty);
}

AST *returnsmt(LexTokenStream *s, SymTable *scope) {
    lex_consume_assert(s, TOK_KRETURN);
    AST *exprast = expr(s, scope);
    lex_consume_assert(s, TOK_SEMICOLON);
    AST **ch = malloc(sizeof(*ch));
    ch[0] = exprast;
    return ast_construct(ch, 1, AST_KRETURN, 0, VAR_VOID);
}

AST *statement(LexTokenStream *s, SymTable *scope) {
    switch (lex_peek(s)->type) {
        case TOK_KINT:
        case TOK_KCHAR:
        case TOK_KSTRUCT:
        case TOK_KVOID:
        case TOK_KENUM:
        case TOK_KLONG:
        case TOK_KSHORT:
            return localvardecl(s, scope);
        case TOK_IDENT: {
            if (lex_peek_n(s, 2)->type == TOK_OPAREN) {
                return funccall(s, scope);
            } else if (lex_peek_n(s, 2)->type == TOK_ASSIGN) {
                return varassign(s, scope);
            } else {
                fprintf(stderr, "Ln %d: Syntax error, expected () or =, got %s\n", lex_peek_n(s, 2)->linenum, toktostr(lex_peek_n(s, 2)->type));
                exit(1);
            }
        }
        case TOK_KPRINT: return printsmt(s, scope);
        case TOK_KIF: return ifsmt(s, scope);
        case TOK_KWHILE: return whilesmt(s, scope);
        case TOK_KFOR: return forsmt(s, scope);
        case TOK_KRETURN: return returnsmt(s, scope);
        default:
            fprintf(stderr, "Ln %d: Syntax error, token %s\n", lex_peek(s)->linenum, toktostr(lex_peek(s)->type));
            exit(1);
    }
}

AST *func_def(LexTokenStream *s, SymTable *scope) {
    SymTable *inside_scope = malloc(sizeof(*inside_scope));
    sym_init(inside_scope, scope);
    inside_scope->base_func_table = 1;

    LexToken *rettype = lex_consume(s);
    VarType retvartype;
    switch (rettype->type) {
        case TOK_KINT:
        case TOK_KCHAR:
        case TOK_KSTRUCT:
        case TOK_KVOID:
        case TOK_KENUM:
        case TOK_KLONG:
        case TOK_KSHORT: 
            retvartype = asttovar(lextoast(rettype->type));
            break;
        default:
            fprintf(stderr, "Ln %d: Syntax error: Expected type identifier, got '%s'", rettype->linenum, toktostr(rettype->type));
            exit(1);
    }
    
    LexToken *t = lex_consume_assert(s, TOK_IDENT);

    lex_consume_assert(s, TOK_OPAREN);
    struct FuncSig sig;
    sig.argtypes = malloc(sizeof(VarType) * 64);
    sig.numargs = 0;
    int cap = 64;

    int curr_stack_offset = 0;
    
    struct ASTList lst;
    ast_list_init(&lst);
    while (lex_peek(s)->type != TOK_CPAREN) {
        LexToken *type = lex_consume(s);
        switch (type->type) {
            case TOK_KINT:
            case TOK_KCHAR:
            case TOK_KSTRUCT:
            case TOK_KVOID:
            case TOK_KENUM:
            case TOK_KLONG:
            case TOK_KSHORT: {
                LexToken *ident = lex_consume_assert(s, TOK_IDENT);
                Symbol *sym = sym_stack_new(inside_scope, ident->str, asttovar(lextoast(type->type)));
                curr_stack_offset += varsize(asttovar(lextoast(type->type)));
                sym->stack_offset = -(curr_stack_offset + 4);

                ast_list_append(&lst, ast_construct(NULL, 0, AST_ARG, sym->ident, VAR_NONE));
                sig.argtypes[sig.numargs++] = asttovar(lextoast(type->type));
                if (sig.numargs >= cap) {
                    sig.argtypes = realloc(sig.argtypes, cap *= 2);
                }

                
                if (lex_peek(s)->type == TOK_CPAREN) break;
                lex_consume_assert(s, TOK_COMMA);
                break;
            }
            default:
                fprintf(stderr, "Ln %d: Syntax error, expected type identifier, got %s\n", type->linenum, toktostr(type->type));
                exit(1);
        }
    }
    lex_consume_assert(s, TOK_CPAREN);
    
    Symbol *sym = sym_find_from_str(scope, t->str);
    if (lex_peek(s)->type == TOK_SEMICOLON) {
        lex_consume_assert(s, TOK_SEMICOLON);
        if (sym == NULL) {
            sym_func_new(scope, t->str, retvartype, sig, 0);
        }
        return NULL;
    } else {
        if (sym != NULL && sym->init_value /*is_defined*/) {
            fprintf(stderr, "Ln %d: Error: function '%s' redefined\n", t->linenum, t->str);
            exit(1);
        }
        AST *body = compoundsmt(s, inside_scope);
        AST *arglist = ast_construct_with_scope(lst.smts, lst.pos, AST_ARGLIST, 0, VAR_NONE, inside_scope);

        AST **children = malloc(sizeof(AST*) * 2);
        children[0] = arglist;
        children[1] = body;
        Symbol *nsym = sym;
        if (sym == NULL) {
            nsym = sym_func_new(scope, t->str, retvartype, sig, 1);
        } else {
            sym->init_value = 1;
        }
        return ast_construct_with_scope(children, 2, AST_FUNC, nsym->ident, VAR_NONE, inside_scope);
    }
}

int ast_gen(AST *ast, LexTokenStream *s, SymTable *scope) {
    AST *a;
    if (lex_peek_n(s, 3)->type == TOK_OPAREN) {
        a = func_def(s, scope);
    } else {
        a = globvardef(s, scope);
        lex_consume_assert(s, TOK_SEMICOLON);
    }
    if (a != NULL) {
        *ast = *a;
        free(a);
    } else {
        *ast = (AST){NULL, 0, AST_INVALID, 0, VAR_NONE, NULL};
    }
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
        case AST_MODULO:     return "AST_MODULO    ";
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
        case AST_KPRINT:     return "AST_KPRINT    ";
        case AST_FUNC:       return "AST_FUNC      ";
        case AST_FUNCCALL:   return "AST_FUNCCALL  ";
        case AST_ARGLIST:    return "AST_ARGLIST   ";
        case AST_ARG:        return "AST_ARG       ";
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
