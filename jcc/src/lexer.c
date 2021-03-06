#include <jtools/reader.h>
#include "../include/lexer.h"
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

void lex_append_token(LexTokenStream *s, LexToken t) {
    *s->pos++ = t;
    if ((unsigned)(s->pos - s->start) >= s->capacity) {
        size_t n = s->pos - s->start;
        s->start = realloc(s->start, sizeof(LexToken) * s->capacity * 2);
        s->capacity *= 2;
        s->pos = s->start + n;
    }
}

void lex_move_token(LexTokenStream *s, LexToken *t) {
    lex_append_token(s, *t);
    t->str = NULL;
    t->whitespace = NULL;
}

const char *keywords[] = {
    "int", "char", "struct", "void", "enum", "long", "short",
    "return", "if", "else", "do", "while", "for", "switch",
    "case", "break", "continue", "typedef",
};

const LexTokenType kword_types[] = {
    TOK_KINT, TOK_KCHAR, TOK_KSTRUCT, TOK_KVOID, TOK_KENUM, TOK_KLONG, TOK_KSHORT,
    TOK_KRETURN, TOK_KIF, TOK_KELSE, TOK_KDO, TOK_KWHILE, TOK_KFOR, TOK_KSWITCH,
    TOK_KCASE, TOK_KBREAK, TOK_KCONTINUE, TOK_KTYPEDEF,
};

const char single_char_tokens[] = {'{', '}', '(', ')', ';', ','};
const LexTokenType single_char_token_types[] = {TOK_OBRACE, TOK_CBRACE, TOK_OPAREN, TOK_CPAREN, TOK_SEMICOLON, TOK_COMMA};

LexTokenType get_keyword(char *s) {
    for (size_t i = 0; i < sizeof(keywords) / sizeof(char*); i++) {
        if (strcmp(s, keywords[i]) == 0) {
            return kword_types[i];
        }
    }
    return TOK_INVALID;
}

LexTokenType get_single_char(Reader *r) {
    char c = reader_peek(r);
    for (unsigned int i = 0; i < sizeof(single_char_tokens); i++) {
        if (single_char_tokens[i] == c) {
            reader_consume(r);
            return single_char_token_types[i];
        }
    }
    return TOK_INVALID;
}

LexTokenType get_double_char(Reader *r, char maybe, LexTokenType ifA, LexTokenType ifB) {
    if (reader_peek(r) == maybe) {
        reader_consume(r);
        return ifB;
    }
    return ifA;
}
Error lex_read_token(Reader *r, LexToken *tok, int ln) {
    size_t cap = 1;
    size_t pos = 0;
    char *sp = malloc(1);
    while (isspace(reader_peek(r))) { 
        char c = reader_consume(r);
        if (reader_bytes_left(r) == 0) {
            tok->type = -1;
            return (Error){0, NULL};
        }
        sp[pos++] = c;
        if (pos >= cap) {
            sp = realloc(sp, cap *= 2);
        }
    }
    sp[pos] = 0;
    
    if (isdigit(reader_peek(r)) || (reader_peek(r) == '-' && isdigit(reader_peek_n(r, 2)))) {
        int64_t i = 0;
        int neg = 0;
        if (reader_peek(r) == '-') {
            neg = 1;
            reader_consume(r);
        }
        while (isdigit(reader_peek(r))) {
            i = i * 10 + (reader_consume(r) - '0');
        }
        if (neg) {
            i = -i;
        }
        *tok = (LexToken){NULL, i, TOK_INT, ln, sp};
    } else if (isalpha(reader_peek(r)) || reader_peek(r) == '_') {
        char *start = r->pos;
        while (isalnum(reader_peek(r)) || reader_peek(r) == '_') {
            reader_consume(r);
        }
        size_t sz = r->pos - start;
        char *str = malloc(sz + 1);
        strncpy(str, start, sz);
        str[sz] = 0;
        LexTokenType kword;
        if ((kword = get_keyword(str)) != TOK_INVALID) {
            *tok = (LexToken){NULL, 0, kword, ln, sp};
        } else {
            *tok = (LexToken){str, 0, TOK_IDENT, ln, sp};
        }
    } else if (reader_peek(r) == '"') {
        reader_consume(r);
        char *start = r->pos;
        while (reader_consume(r) != '"') {
            if (reader_bytes_left(r) <= 0) {
                *tok = (LexToken){NULL, 0, TOK_INVALID, ln, sp};
                return error_construct(1, "EOL whilst scanning literal");
            }
        }
        size_t sz = r->pos - start;
        char *str = malloc(sz);
        strncpy(str, start, sz - 1);
        str[sz - 1] = 0;
        *tok = (LexToken){str, 0, TOK_STR_LIT, ln, sp};
    } else if (reader_consume_if(r, '=')) { *tok = (LexToken){NULL, 0, get_double_char(r, '=', TOK_EQUALS, TOK_EQUALSEQUALS), ln, sp};
    } else if (reader_consume_if(r, '+')) { *tok = (LexToken){NULL, 0, get_double_char(r, '=', TOK_PLUS, TOK_PLUSEQUAL), ln, sp};
    } else if (reader_consume_if(r, '-')) { *tok = (LexToken){NULL, 0, get_double_char(r, '=', TOK_MINUS, TOK_MINUSEQUAL), ln, sp};
    } else if (reader_consume_if(r, '*')) { *tok = (LexToken){NULL, 0, get_double_char(r, '=', TOK_STAR, TOK_STAREQUAL), ln, sp};
    } else if (reader_consume_if(r, '/')) { *tok = (LexToken){NULL, 0, get_double_char(r, '=', TOK_SLASH, TOK_SLASHEQUAL), ln, sp};
    } else if (reader_consume_if(r, '%')) { *tok = (LexToken){NULL, 0, TOK_MODULO, ln, sp};
    } else if (reader_consume_if(r, '>')) { 
        if (reader_peek(r) == '=') { *tok = (LexToken){NULL, 0, TOK_GTE, ln, sp};
        } else {                     *tok = (LexToken){NULL, 0, TOK_GT, ln, sp}; }
    } else if (reader_consume_if(r, '<')) {
        if (reader_peek(r) == '=') { *tok = (LexToken){NULL, 0, TOK_LTE, ln, sp};
        } else {                     *tok = (LexToken){NULL, 0, TOK_LT, ln, sp}; }
    } else {
        LexTokenType a = get_single_char(r);
        *tok = (LexToken){NULL, 0, a, ln, sp};
        if (a == TOK_INVALID) {
            char *buf = malloc(40);
            sprintf(buf, "Invalid character '%c' in input stream", reader_consume(r));
            return (Error){1, buf};
        }
    }
    return (Error){0, NULL};
}

Error lex_tokenise_line(Reader *line, LexTokenStream *s, int ln) {
    if (reader_peek(line) == '#') {
        size_t bytes = reader_bytes_left(line);
        char *str = malloc(bytes + 1);
        strncpy(str, line->start, bytes);
        str[bytes] = 0;
        lex_append_token(s, (LexToken){str, 0, TOK_PREPROC, ln, ""});
        //printf("Alloc'ing str for token %lu in stream %lu\n", (long unsigned int)(s->pos - 1), (long unsigned int)s);

        return (Error){0, NULL};
    }
    while (reader_bytes_left(line) > 0) {
        if (reader_peek(line) == '/' && reader_peek_n(line, 2) == '/') {
            //Is a comment, skip rest of line
            return (Error){0, NULL};
        }
        LexToken t;
        Error e = lex_read_token(line, &t, ln);
        if (e.code != 0) {
            return e;
        }
        if ((int)t.type != -1) {
            lex_append_token(s, t);
        }
    }
    return (Error){0, NULL};
}

Error lex_read_tokens(LexTokenStream *s, Reader *reader) {
    int ln = 1;
    while (reader_bytes_left(reader) > 0) {
        char *line = reader_read_line(reader);
        Reader r;
        reader_construct_from(&r, line);
        lex_tokenise_line(&r, s, ln);
        ln++;
    }
    lex_append_token(s, (LexToken){NULL, 0, TOK_EOF, ln, NULL});
    s->pos = s->start;
    return (Error){0, NULL};
}

void lex_init(LexTokenStream *s) {
    s->start = malloc(sizeof(LexToken) * 4);
    s->pos = s->start;
    s->capacity = 4;
}

LexToken *lex_consume(LexTokenStream *s) {
    //printf("Consuming type %s\n", toktostr(s->pos->type));
    return s->pos++;
}

LexToken *lex_peek(LexTokenStream *s) {
    //printf("Peeking at type %s\n", toktostr(s->pos->type));
    return s->pos;
}

LexToken *lex_peek_n(LexTokenStream *s, int n) {
    return s->pos + n - 1;
}

LexToken *lex_consume_assert(LexTokenStream *s, LexTokenType ty) {
    LexToken *t = lex_consume(s);
    if (t->type != ty) {
        fprintf(stderr, "Ln %d: Syntax error: Expected '%s', but got '%s' instead (value %lu).\n", t->linenum, toktostr(ty), toktostr(t->type), t->i);
        exit(1);
    }
    return t;
}

LexToken *lex_peek_assert(LexTokenStream *s, LexTokenType ty) {
    LexToken *t = lex_peek(s);
    if (t->type != ty) {
        fprintf(stderr, "Ln %d: Syntax error: Expected '%s', but got '%s' instead.\n", t->linenum, toktostr(ty), toktostr(t->type));
        exit(1);
    }
    return t;
}

void lex_put_back(LexTokenStream *s) {
    s->pos--;
}

void lex_free_token(LexToken *t) {
    if (t->str != NULL) {
        free(t->str);
    }
    if (t->whitespace != NULL) {
        free(t->whitespace);
    }
}

void lex_free(LexTokenStream *s) {
    LexToken *t;
    s->pos = s->start;
    do {
        t = lex_consume(s);
        //lex_free_token_n(t, (unsigned long int)s);
        lex_free_token(t);
    } while (t->type != TOK_EOF);
    free(s->start);
}

const char *toktostr(LexTokenType tok) {
    switch (tok) {
        case TOK_INVALID:      return "TOK_INVALID";
        case TOK_INT:          return "TOK_INT";
        case TOK_STR_LIT:      return "TOK_STR_LIT";
        case TOK_IDENT:        return "TOK_IDENT";
        case TOK_PLUS:         return "TOK_PLUS";
        case TOK_MINUS:        return "TOK_MINUS";
        case TOK_STAR:         return "TOK_STAR";
        case TOK_SLASH:        return "TOK_SLASH";
        case TOK_EQUALS:       return "TOK_EQUALS";
        case TOK_EQUALSEQUALS: return "TOK_EQUALSEQUALS";
        case TOK_GT:           return "TOK_GT";
        case TOK_LT:           return "TOK_GT";
        case TOK_GTE:          return "TOK_GTE";
        case TOK_LTE:          return "TOK_LTE";
        case TOK_PLUSEQUAL:    return "TOK_PLUSEQUAL";
        case TOK_MINUSEQUAL:   return "TOK_MINUSEQUAL";
        case TOK_STAREQUAL:    return "TOK_STAREQUAL";
        case TOK_SLASHEQUAL:   return "TOK_SLASHEQUAL";
        case TOK_OPAREN:       return "TOK_OPAREN";
        case TOK_CPAREN:       return "TOK_CPAREN";
        case TOK_OBRACE:       return "TOK_OBRACE";
        case TOK_CBRACE:       return "TOK_CBRACE";
        case TOK_SEMICOLON:    return "TOK_SEMICOLON";
        case TOK_COMMA:        return "TOK_COMMA";
        case TOK_PREPROC:      return "TOK_PREPROC";
        case TOK_AND:          return "TOK_AND";
        case TOK_ANDAND:       return "TOK_ANDAND";
        case TOK_OR:           return "TOK_OR";
        case TOK_OROR:         return "TOK_OROR";
        case TOK_CARAT:        return "TOK_CARAT";
        case TOK_MODULO:       return "TOK_MODULO";
        case TOK_KINT:         return "TOK_KINT";
        case TOK_KCHAR:        return "TOK_KCHAR";
        case TOK_KSTRUCT:      return "TOK_KSTRUCT";
        case TOK_KVOID:        return "TOK_KVOID";
        case TOK_KENUM:        return "TOK_KENUM";
        case TOK_KLONG:        return "TOK_KLONG";
        case TOK_KSHORT:       return "TOK_KSHORT";
        case TOK_KRETURN:      return "TOK_KRETURN";
        case TOK_KIF:          return "TOK_KIF";
        case TOK_KELSE:        return "TOK_KELSE";
        case TOK_KDO:          return "TOK_KDO";
        case TOK_KWHILE:       return "TOK_KWHILE";
        case TOK_KFOR:         return "TOK_KFOR";
        case TOK_KSWITCH:      return "TOK_KSWITCH";
        case TOK_KCASE:        return "TOK_KCASE";
        case TOK_KBREAK:       return "TOK_KBREAK";
        case TOK_KCONTINUE:    return "TOK_KCONTINUE";
        case TOK_KTYPEDEF:     return "TOK_KTYPEDEF";
        case TOK_EOF:          return "TOK_EOF";
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
    s->pos = s->start;
    int last_ln = 0;
    LexToken *t = lex_peek(s);
    while (t->type != TOK_EOF) {
        t = lex_consume(s);
        if (t->type == TOK_EOF) break;
        
        char buf[100];
        if (last_ln != t->linenum) {
            char nbuf[1000];
            int pos = 0;
            for (int i = 0; i < t->linenum - last_ln; i++) {
                pos += sprintf(nbuf + pos, "\n% 3d???", last_ln + i);
            }
            last_ln = t->linenum;
            strcat_alloc(&out, nbuf, &outsz, &outpos);
        }
        strcat_alloc(&out, t->whitespace, &outsz, &outpos);
        
        switch (t->type) {
            case TOK_INT:         sprintf(buf, "%lu", t->i); strcat_alloc(&out, buf, &outsz, &outpos); break;
            case TOK_STR_LIT:     sprintf(buf, "\"%s\"", t->str); strcat_alloc(&out, buf, &outsz, &outpos);break;
            case TOK_IDENT:       sprintf(buf, "%s", t->str); strcat_alloc(&out, buf, &outsz, &outpos); break;
            case TOK_PLUS:        strcat_alloc(&out, "+", &outsz, &outpos); break;
            case TOK_MINUS:       strcat_alloc(&out, "-", &outsz, &outpos); break;
            case TOK_STAR:        strcat_alloc(&out, "*", &outsz, &outpos); break;
            case TOK_SLASH:       strcat_alloc(&out, "/", &outsz, &outpos); break;
            case TOK_EQUALS:      strcat_alloc(&out, "=", &outsz, &outpos); break;
            case TOK_EQUALSEQUALS:strcat_alloc(&out, "==", &outsz, &outpos); break;
            case TOK_GT:          strcat_alloc(&out, ">", &outsz, &outpos); break;
            case TOK_LT:          strcat_alloc(&out, "<", &outsz, &outpos); break;
            case TOK_GTE:         strcat_alloc(&out, ">=", &outsz, &outpos); break;
            case TOK_LTE:         strcat_alloc(&out, "<=", &outsz, &outpos); break;
            case TOK_PLUSEQUAL:   strcat_alloc(&out, "+=", &outsz, &outpos); break;
            case TOK_MINUSEQUAL:  strcat_alloc(&out, "-=", &outsz, &outpos); break;
            case TOK_STAREQUAL:   strcat_alloc(&out, "*=", &outsz, &outpos); break;
            case TOK_SLASHEQUAL:  strcat_alloc(&out, "/=", &outsz, &outpos); break;
            case TOK_OPAREN:      strcat_alloc(&out, "(", &outsz, &outpos); break;
            case TOK_CPAREN:      strcat_alloc(&out, ")", &outsz, &outpos); break;
            case TOK_OBRACE:      strcat_alloc(&out, "{", &outsz, &outpos); break;
            case TOK_CBRACE:      strcat_alloc(&out, "}", &outsz, &outpos); break;
            case TOK_SEMICOLON:   strcat_alloc(&out, ";", &outsz, &outpos); break;
            case TOK_COMMA:       strcat_alloc(&out, ",", &outsz, &outpos); break;
            case TOK_PREPROC:     strcat_alloc(&out, "", &outsz, &outpos); strcat_alloc(&out, t->str, &outsz, &outpos); break;
            case TOK_EOF:         strcat_alloc(&out, "", &outsz, &outpos); break;
            case TOK_INVALID:     strcat_alloc(&out, "~", &outsz, &outpos); break;
            case TOK_AND:         strcat_alloc(&out, "&", &outsz, &outpos); break;
            case TOK_ANDAND:      strcat_alloc(&out, "&&", &outsz, &outpos); break;
            case TOK_OR:          strcat_alloc(&out, "|", &outsz, &outpos); break;
            case TOK_OROR:        strcat_alloc(&out, "||", &outsz, &outpos); break;
            case TOK_CARAT:       strcat_alloc(&out, "^", &outsz, &outpos); break;
            case TOK_MODULO:      strcat_alloc(&out, "%", &outsz, &outpos); break;
            case TOK_KINT:        strcat_alloc(&out, "int", &outsz, &outpos); break;
            case TOK_KCHAR:       strcat_alloc(&out, "char", &outsz, &outpos); break;
            case TOK_KSTRUCT:     strcat_alloc(&out, "struct", &outsz, &outpos); break;
            case TOK_KVOID:       strcat_alloc(&out, "void", &outsz, &outpos); break;
            case TOK_KENUM:       strcat_alloc(&out, "enum", &outsz, &outpos); break;
            case TOK_KLONG:       strcat_alloc(&out, "long", &outsz, &outpos); break;
            case TOK_KSHORT:      strcat_alloc(&out, "short", &outsz, &outpos); break;
            case TOK_KRETURN:     strcat_alloc(&out, "return", &outsz, &outpos); break;
            case TOK_KIF:         strcat_alloc(&out, "if", &outsz, &outpos); break;
            case TOK_KELSE:       strcat_alloc(&out, "else", &outsz, &outpos); break;
            case TOK_KDO:         strcat_alloc(&out, "do", &outsz, &outpos); break;
            case TOK_KWHILE:      strcat_alloc(&out, "while", &outsz, &outpos); break;
            case TOK_KFOR:        strcat_alloc(&out, "for", &outsz, &outpos); break;
            case TOK_KSWITCH:     strcat_alloc(&out, "switch", &outsz, &outpos); break;
            case TOK_KCASE:       strcat_alloc(&out, "case", &outsz, &outpos); break;
            case TOK_KBREAK:      strcat_alloc(&out, "break", &outsz, &outpos); break;
            case TOK_KCONTINUE:   strcat_alloc(&out, "continue", &outsz, &outpos); break;
            case TOK_KTYPEDEF:    strcat_alloc(&out, "typedef", &outsz, &outpos); break;
        }
    }
    out[outpos] = 0;
    return out;
}

//void lex_clone_token(LexToken *t) {
//    if (t->)
//}
