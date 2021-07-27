#include <stdio.h>
#include <stdlib.h>
#include "../include/reader.h"
#include "../include/lexer.h"

const char *toktostr(LexTokenType tok) {
    switch (tok) {
        case TOK_INT:         return "TOK_INT";
        case TOK_IDENT:      return "TOK_IDENT";
        case TOK_ADD:        return "TOK_ADD";
        case TOK_SUB:        return "TOK_SUB";
        case TOK_MUL:        return "TOK_MUL";
        case TOK_DIV:        return "TOK_DIV";
        case TOK_ASSIGN:     return "TOK_ASSIGN";
        case TOK_COMPARE:    return "TOK_COMPARE";
        case TOK_GT:         return "TOK_GT";
        case TOK_LT:         return "TOK_LT";
        case TOK_GTE:        return "TOK_GTE";
        case TOK_LTE:        return "TOK_LTE";
        case TOK_ADD_ASSIGN: return "TOK_ADD_ASSIGN";
        case TOK_SUB_ASSIGN: return "TOK_SUB_ASSIGN";
        case TOK_MUL_ASSIGN: return "TOK_MUL_ASSIGN";
        case TOK_DIV_ASSIGN: return "TOK_DIV_ASSIGN";
        case TOK_OPAREN:     return "TOK_OPAREN";
        case TOK_CPAREN:     return "TOK_CPAREN";
        case TOK_OBRACE:     return "TOK_OBRACE";
        case TOK_CBRACE:     return "TOK_CBRACE";
        case TOK_SEMICOLON:  return "TOK_SEMICOLON";
        case TOK_EOF:        return "TOK_EOF";
        case TOK_INVALID:    return "TOK_INVALID";
    }
    return NULL;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "%s: too few arguments (expected filename)\n", argv[0]);
        return 1;
    }

    Reader r;
    int err = reader_construct(&r, argv[1]);
    if (err != 0) {
        fprintf(stderr, "%s: cannot read file %s: no such file\n", argv[0], argv[1]);
        return err;
    }

    LexTokenStream s;
    int lex_err = lex_read_tokens(&s, &r);
    if (lex_err != 0) {
        fprintf(stderr, "%s: Lexer returned non-zero status %d\n", argv[0], lex_err);
        return lex_err;
    }
    LexToken *t;
    do {
        t = lex_consume(&s);
        printf("type: %s, i: %lu, str: %s\n", toktostr(t->type), t->i, t->str);
    } while (t->type != TOK_EOF);

    lex_free(&s);
    reader_free(&r);
    return 0;
}