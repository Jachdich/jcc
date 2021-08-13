#ifndef __LEXER_H
#define __LEXER_H
#include "../include/reader.h"
#include "../include/error.h"
#include <stdint.h>

enum LexTokenType {
    TOK_INVALID = 0,
    TOK_INT,
    TOK_STR_LIT,
    TOK_IDENT,
    TOK_ADD,
    TOK_SUB,
    TOK_MUL,
    TOK_DIV,
    TOK_ASSIGN,
    TOK_COMPARE,
    TOK_GT,
    TOK_LT,
    TOK_GTE,
    TOK_LTE,
    TOK_ADD_ASSIGN,
    TOK_SUB_ASSIGN,
    TOK_MUL_ASSIGN,
    TOK_DIV_ASSIGN,
    TOK_OPAREN,
    TOK_CPAREN,
    TOK_OBRACE,
    TOK_CBRACE,
    TOK_SEMICOLON,
    TOK_COMMA,
    TOK_PREPROC,
    TOK_AND,
    TOK_ANDAND,
    TOK_OR,
    TOK_OROR,

    TOK_KINT,
    TOK_KCHAR,
    TOK_KSTRUCT,
    TOK_KVOID,
    TOK_KENUM,
    TOK_KLONG,
    TOK_KSHORT,
    TOK_KRETURN,
    TOK_KIF,
    TOK_KELSE,
    TOK_KDO,
    TOK_KWHILE,
    TOK_KFOR,
    TOK_KSWITCH,
    TOK_KCASE,
    TOK_KBREAK,
    TOK_KCONTINUE,
    TOK_KTYPEDEF,

    //TEMP
    TOK_KPRINT,
    
    TOK_EOF,
};

typedef enum LexTokenType LexTokenType;

struct LexToken {
    char *str;
    size_t i;
    LexTokenType type;
};

typedef struct LexToken LexToken;

struct LexTokenStream {
    LexToken *start;
    LexToken *pos;
    size_t capacity;
};

typedef struct LexTokenStream LexTokenStream;


Error lex_read_tokens(LexTokenStream *s, Reader *reader);
LexToken *lex_consume(LexTokenStream *s);
LexToken *lex_peek(LexTokenStream *s);
void lex_free(LexTokenStream *s);
void lex_free_token(LexToken *t);
Error lex_tokenise_line(Reader *line, LexTokenStream *s);
void lex_print_tokens(LexTokenStream *s);
void lex_append_token(LexTokenStream *s, LexToken t);
void lex_init(LexTokenStream *s);
char *lex_reconstruct_src(LexTokenStream *s);
void lex_extend_tokens(LexTokenStream *s, LexTokenStream *src);
//LexToken lex_clone_token(LexToken *t);
const char *toktostr(LexTokenType tok);

LexToken *lex_consume_assert(LexTokenStream *s, LexTokenType ty);
LexToken *lex_peek_assert(LexTokenStream *s, LexTokenType ty);
#endif
