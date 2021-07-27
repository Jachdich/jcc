#include "../include/reader.h"
#include "../include/lexer.h"
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

LexToken lex_read_token(Reader *r) {
    while (isspace(reader_peek(r))) { 
        reader_consume(r);
    }
    
    if (isdigit(reader_peek(r))) {
        int64_t i = 0;
        while (isdigit(reader_peek(r))) {
            i = i * 10 + (reader_consume(r) - '0');
        }
        return (LexToken){NULL, i, TOK_INT};
    } else if (isalpha(reader_peek(r)) || reader_peek(r) == '_') {
        char *start = r->pos;
        while (isalnum(reader_peek(r)) || reader_peek(r) == '_') {
            reader_consume(r);
        }
        size_t sz = r->pos - start;
        char *str = malloc(sz + 1);
        strncpy(str, start, sz);
        str[sz] = 0;
        return (LexToken){str, 0, TOK_IDENT};
    } else if (reader_peek(r) == '=') {
        reader_consume(r);
        if (reader_peek(r) == '=') {
            reader_consume(r);
            return (LexToken){NULL, 0, TOK_COMPARE};
        } else {
            return (LexToken){NULL, 0, TOK_ASSIGN};
        }
    } else if (reader_peek(r) == ';') {
        reader_consume(r);
        return (LexToken){NULL, 0, TOK_SEMICOLON};
    } else if (reader_peek(r) == '{') {
        reader_consume(r);
        return (LexToken){NULL, 0, TOK_OBRACE};
    } else if (reader_peek(r) == '}') {
        reader_consume(r);
        return (LexToken){NULL, 0, TOK_CBRACE};
    } else if (reader_peek(r) == '(') {
        reader_consume(r);
        return (LexToken){NULL, 0, TOK_OPAREN};
    } else if (reader_peek(r) == ')') {
        reader_consume(r);
        return (LexToken){NULL, 0, TOK_CPAREN};
    } else {
        reader_consume(r);
        return (LexToken){NULL, 0, TOK_INVALID};
    }
}

int lex_tokenise_line(Reader *line, LexTokenStream *s) {
    printf("%s\n", line->start);
    while (reader_bytes_left(line) > 0) {
        (*s->pos) = lex_read_token(line);
        s->pos++;
        if ((unsigned)(s->pos - s->start) >= s->capacity) {
            size_t n = s->pos - s->start;
            s->start = realloc(s->start, sizeof(LexToken) * s->capacity * 2);
            s->capacity *= 2;
            s->pos = s->start + n;
        }
    }
    return 0;
}

int lex_read_tokens(LexTokenStream *s, Reader *reader) {
    s->start = malloc(sizeof(LexToken) * 64);
    s->pos = s->start;
    s->capacity = 64;
    while (reader_bytes_left(reader) > 0) {
        char *line = reader_read_line(reader);
        Reader r;
        reader_construct_from(&r, line);
        lex_tokenise_line(&r, s);
    }
    LexToken token;
    token.str = NULL;
    token.type = TOK_EOF;
    *(s->pos) = token;
    s->pos = s->start;
    return 0;
}

LexToken *lex_consume(LexTokenStream *s) {
    return s->pos++;
}

LexToken *lex_peek(LexTokenStream *s) {
    return s->pos;
}

void lex_free_token(LexToken *t) {
    if (t->str != NULL) free(t->str);
}

void lex_free(LexTokenStream *s) {
    LexToken *t;
    s->pos = s->start;
    do {
        t = lex_consume(s);
        lex_free_token(t);
    } while (t->type != TOK_EOF);
    free(s->start);
}