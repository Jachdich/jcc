#include "../include/reader.h"
#include "../include/lexer.h"
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

void lex_append_token(LexTokenStream *s, LexToken t) {
    (*s->pos) = t;
    s->pos++;
    if ((unsigned)(s->pos - s->start) >= s->capacity) {
        size_t n = s->pos - s->start;
        s->start = realloc(s->start, sizeof(LexToken) * s->capacity * 2);
        s->capacity *= 2;
        s->pos = s->start + n;
    }
}

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
    } else if (reader_peek(r) == '"') {
        reader_consume(r);
        char *start = r->pos;
        while (reader_consume(r) != '"') {
            if (reader_bytes_left(r) <= 0) {
                fprintf(stderr, "EOL whilst scanning literal");
                return (LexToken){NULL, 0, TOK_INVALID};
            }
        }
        size_t sz = r->pos - start;
        char *str = malloc(sz);
        strncpy(str, start, sz - 1);
        str[sz - 1] = 0;
        return (LexToken){str, 0, TOK_STR_LIT};
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
    } else if (reader_peek(r) == ',') {
        reader_consume(r);
        return (LexToken){NULL, 0, TOK_COMMA};
    } else {
        reader_consume(r);
        return (LexToken){NULL, 0, TOK_INVALID};
    }
}

int lex_tokenise_line(Reader *line, LexTokenStream *s) {
    if (reader_peek(line) == '#') {
        size_t bytes = reader_bytes_left(line);
        char *str = malloc(bytes + 1);
        strncpy(str, line->start, bytes);
        str[bytes] = 0;
        lex_append_token(s, (LexToken){str, 0, TOK_PREPROC});
        return 0;
    }
    while (reader_bytes_left(line) > 0) {
        lex_append_token(s, lex_read_token(line));
    }
    return 0;
}

int lex_read_tokens(LexTokenStream *s, Reader *reader) {
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

void lex_init(LexTokenStream *s) {
    s->start = malloc(sizeof(LexToken) * 64);
    s->pos = s->start;
    s->capacity = 64;
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

const char *toktostr(LexTokenType tok) {
    switch (tok) {
        case TOK_INT:        return "TOK_INT       ";
        case TOK_STR_LIT:    return "TOK_STR_LIT   ";
        case TOK_IDENT:      return "TOK_IDENT     ";
        case TOK_ADD:        return "TOK_ADD       ";
        case TOK_SUB:        return "TOK_SUB       ";
        case TOK_MUL:        return "TOK_MUL       ";
        case TOK_DIV:        return "TOK_DIV       ";
        case TOK_ASSIGN:     return "TOK_ASSIGN    ";
        case TOK_COMPARE:    return "TOK_COMPARE   ";
        case TOK_GT:         return "TOK_GT        ";
        case TOK_LT:         return "TOK_LT        ";
        case TOK_GTE:        return "TOK_GTE       ";
        case TOK_LTE:        return "TOK_LTE       ";
        case TOK_ADD_ASSIGN: return "TOK_ADD_ASSIGN";
        case TOK_SUB_ASSIGN: return "TOK_SUB_ASSIGN";
        case TOK_MUL_ASSIGN: return "TOK_MUL_ASSIGN";
        case TOK_DIV_ASSIGN: return "TOK_DIV_ASSIGN";
        case TOK_OPAREN:     return "TOK_OPAREN    ";
        case TOK_CPAREN:     return "TOK_CPAREN    ";
        case TOK_OBRACE:     return "TOK_OBRACE    ";
        case TOK_CBRACE:     return "TOK_CBRACE    ";
        case TOK_SEMICOLON:  return "TOK_SEMICOLON ";
        case TOK_COMMA:      return "TOK_COMMA     ";
        case TOK_PREPROC:    return "TOK_PREPROC   ";
        case TOK_EOF:        return "TOK_EOF       ";
        case TOK_INVALID:    return "TOK_INVALID   ";
    }
    return NULL;
}

void lex_print_tokens(LexTokenStream *s) {
    LexToken *t;
    do {
        t = lex_consume(s);
        printf("type: %s i: %lu, str: %s\n", toktostr(t->type), t->i, t->str);
    } while (t->type != TOK_EOF);
}

void lex_extend_tokens(LexTokenStream *s, LexTokenStream *src) {
    LexToken *t = lex_peek(src);
    while (t->type != TOK_EOF) {
        t = lex_consume(src);
        if (t->type != TOK_EOF) {
            lex_append_token(s, *t);
            t->str = NULL;
        }
    }
}

void strcat_alloc(char **dest, const char *src, size_t *sz, size_t *pos) {
    if (*pos + strlen(src) < *sz) {
        strcpy(*dest + *pos, src);
        *pos += strlen(src);
    } else {
        *dest = realloc(*dest, *sz * 2);
        *sz *= 2;
        strcpy(*dest + *pos, src);
        *pos += strlen(src);
    }
}

char *lex_reconstruct_src(LexTokenStream *s) {
    char *out = malloc(64);
    size_t outsz = 64;
    size_t outpos = 0;
    LexToken *t;
    s->pos = s->start;
    do {
        t = lex_consume(s);
        char buf[100];
        switch (t->type) {
            case TOK_INT:        sprintf(buf, "%lu", t->i); strcat_alloc(&out, buf, &outsz, &outpos); break;
            case TOK_STR_LIT:    sprintf(buf, "\"%s\"", t->str); strcat_alloc(&out, buf, &outsz, &outpos);break;
            case TOK_IDENT:      sprintf(buf, "%s ", t->str); strcat_alloc(&out, buf, &outsz, &outpos); break;
            case TOK_ADD:        strcat_alloc(&out, "+", &outsz, &outpos); break;
            case TOK_SUB:        strcat_alloc(&out, "-", &outsz, &outpos); break;
            case TOK_MUL:        strcat_alloc(&out, "*", &outsz, &outpos); break;
            case TOK_DIV:        strcat_alloc(&out, "/", &outsz, &outpos); break;
            case TOK_ASSIGN:     strcat_alloc(&out, "= ", &outsz, &outpos); break;
            case TOK_COMPARE:    strcat_alloc(&out, "== ", &outsz, &outpos); break;
            case TOK_GT:         strcat_alloc(&out, "> ", &outsz, &outpos); break;
            case TOK_LT:         strcat_alloc(&out, "< ", &outsz, &outpos); break;
            case TOK_GTE:        strcat_alloc(&out, ">= ", &outsz, &outpos); break;
            case TOK_LTE:        strcat_alloc(&out, "<= ", &outsz, &outpos); break;
            case TOK_ADD_ASSIGN: strcat_alloc(&out, "+= ", &outsz, &outpos); break;
            case TOK_SUB_ASSIGN: strcat_alloc(&out, "-= ", &outsz, &outpos); break;
            case TOK_MUL_ASSIGN: strcat_alloc(&out, "*= ", &outsz, &outpos); break;
            case TOK_DIV_ASSIGN: strcat_alloc(&out, "/= ", &outsz, &outpos); break;
            case TOK_OPAREN:     strcat_alloc(&out, "(", &outsz, &outpos); break;
            case TOK_CPAREN:     strcat_alloc(&out, ")", &outsz, &outpos); break;
            case TOK_OBRACE:     strcat_alloc(&out, "{\n", &outsz, &outpos); break;
            case TOK_CBRACE:     strcat_alloc(&out, "}", &outsz, &outpos); break;
            case TOK_SEMICOLON:  strcat_alloc(&out, ";\n", &outsz, &outpos); break;
            case TOK_COMMA:      strcat_alloc(&out, ", ", &outsz, &outpos); break;
            case TOK_PREPROC:    strcat_alloc(&out, "\n", &outsz, &outpos); strcat_alloc(&out, t->str, &outsz, &outpos); break;
            case TOK_EOF:        strcat_alloc(&out, "", &outsz, &outpos); break;
            case TOK_INVALID:    strcat_alloc(&out, "~", &outsz, &outpos); break;
        }
    } while (t->type != TOK_EOF);
    out[outpos] = 0;
    return out;
}

//void lex_clone_token(LexToken *t) {
//    if (t->)
//}
