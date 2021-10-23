#include "../include/parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef Error (*ExprFunc)(AST*, LexTokenStream*);

int is_unary(LexTokenType ty) {
    switch (ty) {
        case TOK_MINUS:
        case TOK_STAR:
        case TOK_AND:
            return 1;
        default:
            return 0;
    }
}

int is_type(LexTokenType ty) {
    switch (ty) {
        case TOK_KINT:
        case TOK_KCHAR:
        case TOK_KSHORT:
        case TOK_KLONG:
            return 1;
        default:
            return 0;
    }
}

ASTType lex_to_ast_binexpr(LexTokenType t) {
    switch (t) {
        case TOK_PLUS:  return AST_ADD;
        case TOK_MINUS: return AST_SUB;
        case TOK_STAR:  return AST_MUL;
        case TOK_SLASH: return AST_DIV;
        case TOK_EQUALSEQUALS: return AST_COMPARE;
        default: return AST_INVALID;
    }
}

ASTType lex_to_ast_kword(LexTokenType t) {
    switch (t) {
        case TOK_KINT: return AST_KINT;
        case TOK_KCHAR: return AST_KCHAR;
        case TOK_KSTRUCT: return AST_KSTRUCT;
        case TOK_KVOID: return AST_KVOID;
        case TOK_KENUM: return AST_KENUM;
        case TOK_KLONG: return AST_KLONG;
        case TOK_KSHORT: return AST_KSHORT;
        case TOK_KRETURN: return AST_KRETURN;
        case TOK_KIF: return AST_KIF;
        case TOK_KELSE: return AST_KELSE;
        case TOK_KDO: return AST_KDO;
        case TOK_KWHILE: return AST_KWHILE;
        case TOK_KFOR: return AST_KFOR;
        case TOK_KSWITCH: return AST_KSWITCH;
        case TOK_KCASE: return AST_KCASE;
        case TOK_KBREAK: return AST_KBREAK;
        case TOK_KCONTINUE: return AST_KCONTINUE;
        case TOK_KTYPEDEF: return AST_KTYPEDEF;
        default: return AST_INVALID;
    }
}

int is_in_array(LexTokenType t, LexTokenType *a, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] == t) return 1;
    }
    return 0;
}

Error expr(AST *o, LexTokenStream *s);

Error number(AST *o, LexTokenStream *s) {
    LexToken *t = lex_consume_assert(s, TOK_INT);
    *o = (AST){NULL, 0, AST_INT_LIT, {t->i}, VAR_INT};
    return (Error){0, NULL};
}

Error unexpr(AST *o, LexTokenStream *s) {
    (void)o;
    (void)s;
    return (Error){0, NULL};
}

Error arbitrary_expr(AST *o, LexTokenStream *s, ExprFunc f, LexTokenType *types, int num) {
    AST lhs;
    Error ne = f(&lhs, s);
    if (ne.code != 0) return ne;
    LexTokenType type = lex_peek(s)->type;
    while (is_in_array(type, types, num)) {
        LexToken *oper = lex_consume(s);
        AST res;
        res.children = malloc(sizeof(AST) * 2);
        res.children_n = 2;
        res.type = lex_to_ast_binexpr(oper->type); 
        res.children[0] = lhs;
        res.i = 0;
        res.vtype = VAR_INT;
        Error ne = f(res.children + 1, s);
        if (ne.code != 0) return ne;
        lhs = res;
        type = lex_peek(s)->type;
    }
    *o = lhs;
    return (Error){0, NULL};
}

Error mulexpr(AST *o, LexTokenStream *s) {
    LexTokenType acceptable_tokens[] = {TOK_STAR, TOK_SLASH};
    return arbitrary_expr(o, s, &number, acceptable_tokens, 2);
}

Error addexpr(AST *o, LexTokenStream *s) {
    LexTokenType acceptable_tokens[] = {TOK_PLUS, TOK_MINUS};
    return arbitrary_expr(o, s, &mulexpr, acceptable_tokens, 2);
}

Error binexpr(AST *o, LexTokenStream *s) {
    LexTokenType acceptable_tokens[] = {TOK_EQUALSEQUALS, TOK_GT, TOK_GTE, TOK_LT, TOK_LTE};
    return arbitrary_expr(o, s, &addexpr, acceptable_tokens, 5);
}

Error expr(AST *o, LexTokenStream *s) {
    if (is_unary(lex_peek(s)->type)) {
        return unexpr(o, s);
    } else {
        return binexpr(o, s);
    }
    return (Error){0, NULL};
}

Error locdef(AST *o, LexTokenStream *s) {
    LexToken *type = lex_consume(s);
    if (!is_type(type->type)) {
        char *buf = malloc(64 + strlen(toktostr(type->type)));
        sprintf(buf, "Ln %d: Error: expected type identifier, got %s (%s)\n", type->linenum, toktostr(type->type), type->str);
        return (Error){1, buf};
    }
    LexToken *name = lex_consume_assert(s, TOK_IDENT);
    AST a;
    a.type = AST_LOCVARDEF;
    a.children = malloc(sizeof(*a.children) * 3);
    a.children[0] = (AST){NULL, 0, lex_to_ast_kword(type->type), {0}, VAR_NONE};
    a.children[1] = (AST){NULL, 0, AST_IDENT, {.s = name->str}, VAR_NONE};
    a.children_n = 2;
    *o = a;
   if (lex_peek(s)->type == TOK_EQUALS) {
        lex_consume_assert(s, TOK_EQUALS);
        Error ne = expr(a.children + 2, s);
        lex_consume_assert(s, TOK_SEMICOLON);
        o->children_n = 3;
        return ne;
    } else {
        //TODO should I actually do this here?
        lex_consume_assert(s, TOK_SEMICOLON);
        return (Error){0, NULL};
    }
}

Error lv(AST *o, LexTokenStream *s) {
    LexToken *name = lex_consume_assert(s, TOK_IDENT);
    *o = (AST){NULL, 0, AST_LVIDENT, {.s=name->str}, VAR_NONE};
    return (Error){0, NULL};
}

Error varasign(AST *o, LexTokenStream *s) {
    Error ne;
    AST *children = malloc(sizeof(*children) * 2);
    ne = lv(children, s);
    if (ne.code) return ne;
    lex_consume_assert(s, TOK_EQUALS);
    ne = expr(children + 1, s);
    if (ne.code) return ne;
    lex_consume_assert(s, TOK_SEMICOLON);
    *o = (AST){ children, 2, AST_ASSIGN, {0}, VAR_NONE };
    return (Error){0, NULL};
}
 
Error statement(AST *o, LexTokenStream *s) {
    if (is_type(lex_peek(s)->type)) {
        return locdef(o, s);
    } else {
        return varasign(o, s);
    }
}

Error compoundsmt(AST *o, LexTokenStream *s) {
    lex_consume_assert(s, TOK_OBRACE);

    AST *smts = malloc(sizeof(*smts) * 4);
    size_t cap = 4;
    size_t len = 0;
    while (lex_peek(s)->type != TOK_CBRACE) {
        Error ne = statement(smts + len, s);
        if (ne.code) { free(smts); return ne; }
        if (len >= cap) smts = realloc(smts, (cap *= 2) * sizeof(*smts));
        len++;
    }
    lex_consume_assert(s, TOK_CBRACE);
    *o = (AST){ smts, len, AST_PROG, {0}, VAR_NONE };
    return (Error){0, NULL};
}

Error funcdef(AST *o, LexTokenStream *s) {
    LexToken *rettype = lex_consume(s);
    LexToken *name = lex_consume(s);
    lex_consume_assert(s, TOK_OPAREN);
    lex_consume_assert(s, TOK_CPAREN);
    AST body;
    Error ne = compoundsmt(&body, s);
    if (ne.code != 0) {
        return ne;
    }
    *o = body;
    return (Error){0, NULL};
}

Error globdef(AST *o, LexTokenStream *s) {
    return (Error){0, NULL};
}

Error ast_gen(AST *ast, LexTokenStream *s) {
    AST a;
    Error ret;
    if (is_type(lex_peek(s)->type) && lex_peek_n(s, 2)->type == TOK_IDENT && lex_peek_n(s, 3)->type == TOK_OPAREN) {
        ret = funcdef(&a, s);
    } else if (is_type(lex_peek(s)->type) && lex_peek_n(s, 2)->type == TOK_IDENT && lex_peek_n(s, 3)->type == TOK_EQUALS) {
        ret = globdef(&a, s);
    }
    if (ret.code == 0) {
        *ast = a;
    } else {
        *ast = (AST){NULL, 0, AST_INVALID, {0}, VAR_NONE};
        printf("Parser encountered an error (code %d): %s\n", ret.code, ret.message);
    }
    return ret;
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
        case AST_FUNC:       return "AST_FUNC      ";
        case AST_FUNCCALL:   return "AST_FUNCCALL  ";
        case AST_ARGLIST:    return "AST_ARGLIST   ";
        case AST_ARG:        return "AST_ARG       ";
        case AST_DEREF:      return "AST_DEREF     ";
        case AST_REF:        return "AST_REF       ";
        case AST_LOCVARDEF:  return "AST_LOCVARDEF ";
    }
    return "(AST TOKEN NOT RECOGNISED)";
}

const char *vartostr(VarType ty) {
    switch (ty) {
        case VAR_INT: return "int";
        case VAR_CHAR: return "char";
        case VAR_LONG: return "long";
        case VAR_SHORT: return "short";
        case VAR_VOID: return "void";
        case VAR_STRUCT: return "struct";
        case VAR_ENUM: return "enum";
        case VAR_NONE: return "none";
        default: return "vartype out of bounds";
    }
}

void ast_print_internal(AST *ast, int n) {
    const char *type = asttypetostr(ast->type);
    for (int i = 0; i < n; i++) {
        printf("    ");
    }
    printf("%s %lu %s\n", type, ast->i, vartostr(ast->vtype));
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
