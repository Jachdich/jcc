#include "../include/preprocessor.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

struct StrMap {
    char **keys;
    char **vals;
    size_t capacity;
    size_t size;
};
typedef struct StrMap StrMap;

void map_init(StrMap *m) {
    m->keys = malloc(sizeof(char*) * 64);
    m->vals = malloc(sizeof(char*) * 64);
    m->capacity = 64;
    m->size = 0;
}

void map_free(StrMap *m) {
    for (size_t i = 0; i < m->size; i++) {
        free(m->keys[i]);
        free(m->vals[i]);
    }
    free(m->keys);
    free(m->vals);
}

char *map_find(StrMap *m, char *key) {
    for (size_t i = 0; i < m->size; i++) {
        if (strcmp(key, m->keys[i]) == 0) {
            return m->vals[i];
        }
    }
    return NULL;
}

void map_replace(StrMap *m, char *key, char *val) {
    for (size_t i = 0; i < m->size; i++) {
        if (strcmp(key, m->keys[i]) == 0) {
            m->vals[i] = val;
        }
    }
}

void map_put(StrMap *m, char *key, char *val) {
    m->keys[m->size] = key;
    m->vals[m->size] = val;
    m->size++;
    if (m->size >= m->capacity) {
        m->keys = realloc(m->keys, sizeof(char*) * m->capacity * 2);
        m->vals = realloc(m->vals, sizeof(char*) * m->capacity * 2);
        m->capacity *= 2;
    }
}

char *get_ident(char **str) {
    while (**str == ' ') { (*str)++; };
    char *start = *str;
    while (isalnum(**str) || **str == '_') {
        (*str)++;
    }
    size_t sz = *str - start;
    char *ident = malloc(sz + 1);
    strncpy(ident, start, sz);
    ident[sz] = 0;
    return ident;
}

Error read_until_closing(LexTokenStream *input, LexTokenStream *out) {
    int opening = 1;
    int closing = 0;
    LexToken *t = lex_peek(input);
    while (opening != closing && t->type != TOK_EOF) {
        LexToken *t = lex_consume(input);
        if (t->type == TOK_PREPROC) {
            if (strncmp(t->str, "#endif", 6) == 0) {
                closing++;
            } else if (strncmp(t->str, "#ifdef", 6) == 0 || strncmp(t->str, "#ifndef", 7) == 0) {
                opening++;
            }
        }
        if (opening != closing) {
            lex_append_token(out, *t);
        }
        t->str = NULL;
    }
    if (t->type == TOK_EOF) {
        return error_construct(1, "Error: unclosed #if");
    }
    lex_append_token(out, (LexToken){NULL, 0, TOK_EOF});
    return (Error){0, NULL};
}

Error preprocess_tokens_internal(LexTokenStream *input, StrMap *defines);

Error preprocess_tokens(LexTokenStream *input) {
    StrMap defines;
    map_init(&defines);
    Error e =  preprocess_tokens_internal(input, &defines);
    map_free(&defines);
    return e;
}

Error preprocess_tokens_internal(LexTokenStream *input, StrMap *defines) {
    input->pos = input->start;
    LexToken *tok;
    LexTokenStream out;
    lex_init(&out);
    while ((tok = lex_consume(input))->type != TOK_EOF) {
        if (tok->type == TOK_PREPROC) {
            if (strncmp(tok->str, "#define", 7) == 0) {
                char *str = tok->str + 7;
                //TODO this accepts zero spaces after #define
                char *ident = get_ident(&str);
                while (*str == ' ') { str++; };

                char *val = malloc(strlen(str) + 1);
                strcpy(val, str);
                if (map_find(defines, ident) != NULL) {
                    char *orig = map_find(defines, ident);
                    map_replace(defines, ident, val);
                    free(orig);
                } else {
                    map_put(defines, ident, val);
                }
            } else if (strncmp(tok->str, "#ifdef", 6) == 0) {
                char *str = tok->str + 6;
                char *ident = get_ident(&str);
                LexTokenStream between;
                lex_init(&between);
                Error err = read_until_closing(input, &between);
                if (err.status_code != 0) return err;
                between.pos = between.start;
                if (map_find(defines, ident) != NULL) {
                    preprocess_tokens_internal(&between, defines);
                    lex_extend_tokens(&out, &between);
                }
                free(ident);
            } else if (strncmp(tok->str, "#ifndef", 6) == 0) {
                char *str = tok->str + 7;
                char *ident = get_ident(&str);
                LexTokenStream between;
                lex_init(&between);
                Error err = read_until_closing(input, &between);
                if (err.status_code != 0) return err;
                between.pos = between.start;
                if (map_find(defines, ident) == NULL) {
                    preprocess_tokens_internal(&between, defines);
                    lex_extend_tokens(&out, &between);
                }
                free(ident);
            }
        } else if (tok->type == TOK_IDENT) {
            char *found = map_find(defines, tok->str);
            if (found != NULL) {
                LexTokenStream s;
                lex_init(&s);
                Reader r;
                reader_construct_from(&r, found);
                lex_tokenise_line(&r, &s);
                lex_append_token(&s, (LexToken){NULL, 0, TOK_EOF});
                s.pos = s.start;
                //lex_print_tokens(&s);
                LexToken *t = lex_peek(&s);
                while (t->type != TOK_EOF) {
                    t = lex_consume(&s);
                    if (t->type != TOK_EOF) {
                        lex_append_token(&out, *t);
                        t->str = NULL;
                    }
                }
                lex_free(&s);
            } else {
                lex_append_token(&out, *tok);
                tok->str = NULL;
            }
        } else {
            lex_append_token(&out, *tok);
            tok->str = NULL;
        }
    }
    lex_free(input);
    lex_append_token(&out, (LexToken){NULL, 0, TOK_EOF});
    input->start = out.start;
    input->pos = out.start;
    input->capacity = out.capacity;
    return (Error){0, NULL};
}

char *preprocess_includes(Reader *input) {
    char *output = malloc(reader_bytes_left(input) + 1 /*for null term*/);
    size_t curr_bytes = reader_bytes_left(input);
    char *ptr = output;
    while (reader_bytes_left(input) > 0) {
        char *line = reader_read_line(input);
        if (strncmp(line, "#include", 8) == 0) {
            //just allocate the maximum size the filename *could* be, because I am lazy
            char *fname = malloc(strlen(line) - 8);
            char *pos = line + 7;
            char delim;
            while (*++pos == ' ');
            delim = *pos++;
            if (delim != '"' && delim != '<') {
                fprintf(stderr, "Syntax error: expected '\"' or '<' after '#include'\nat %s\n", line);
                free(output);
                return NULL;
            }
            if (delim == '<') delim = '>';
            char *start = pos;
            while (*++pos != delim);
            strncpy(fname, start, pos - start);
            fname[pos - start] = 0;

            Reader r;
            int err = reader_construct(&r, fname);
            free(fname);
            if (err != 0) {
                fprintf(stderr, "%s: no such file or directory\n", line);
                free(output);
                return NULL;
            }
            
            size_t delta = ptr - output;
            size_t reader_bytes = reader_bytes_left(&r);
            output = realloc(output, curr_bytes + reader_bytes + 1 /*for \n*/);
            curr_bytes += reader_bytes;
            ptr = output + delta;
            strncpy(ptr, r.start, reader_bytes);
            reader_free(&r);
            ptr += reader_bytes;

            //add a \n in case the file doesn't already have one, otherwise bad things happen
            *ptr++ = '\n'; 
            
        } else {
            strcpy(ptr, line);
            ptr += strlen(line) + 1;
            *(ptr - 1) = '\n';
        }
    }
    *ptr = 0;
    return output;
}
