#ifndef __PARSER_H
#define __PARSER_H
#include "../include/lexer.h"
struct AST;
typedef struct AST AST;

enum ASTType {
    AST_INVALID,
    AST_INT_LIT,
    AST_STR_LIT,
    AST_IDENT,
    AST_LVIDENT,
    AST_ADD,
    AST_SUB,
    AST_MUL,
    AST_DIV,
    AST_ASSIGN,
    AST_COMPARE,
    AST_GT,
    AST_LT,
    AST_GTE,
    AST_LTE,
    AST_ADD_ASSIGN,
    AST_SUB_ASSIGN,
    AST_MUL_ASSIGN,
    AST_DIV_ASSIGN,
    AST_AND,
    AST_ANDAND,
    AST_OR,
    AST_OROR,
    AST_MODULO,
    AST_DEREF,
    AST_REF,
    
    AST_KINT,
    AST_KCHAR,
    AST_KSTRUCT,
    AST_KVOID,
    AST_KENUM,
    AST_KLONG,
    AST_KSHORT,
    AST_KRETURN,
    AST_KIF,
    AST_KELSE,
    AST_KDO,
    AST_KWHILE,
    AST_KFOR,
    AST_KSWITCH,
    AST_KCASE,
    AST_KBREAK,
    AST_KCONTINUE,
    AST_KTYPEDEF,

    AST_DECL,
    AST_DECL_VAL,

    AST_PROG,
    AST_FUNC,
    AST_FUNCCALL,
    AST_ARGLIST,
    AST_ARG,

    AST_LOCVARDEF,
};

enum VarType {
    VAR_INT,
    VAR_CHAR,
    VAR_STRUCT,
    VAR_VOID,
    VAR_ENUM,
    VAR_LONG,
    VAR_SHORT,
    VAR_NONE,
};


typedef enum ASTType ASTType;
typedef enum VarType VarType;

struct AST {
    AST *children;
    size_t children_n;
    ASTType type;
    union {
        size_t i;
        char *s;
    };
    VarType vtype;
};

Error ast_gen(AST *ast, LexTokenStream *s);
void ast_print(AST *ast);
void ast_free(AST *ast);
const char *asttypetostr(ASTType ty);
#endif
