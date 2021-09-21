#include "../include/reader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int reader_construct(Reader *r, char *fname) {
    FILE *fp = fopen(fname, "r");
    if (fp == NULL) {
        return 1;
    }
    fseek(fp, 0, SEEK_END);
    size_t fsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char *contents = malloc(fsize + 1);
    fread(contents, 1, fsize, fp);
    contents[fsize] = 0;
    fclose(fp);
    r->start = contents;
    r->pos = contents;
    r->len = fsize;
    return 0;
}

int reader_construct_from(Reader *r, char *buf) {
    r->start = buf;
    r->pos = buf;
    r->len = strlen(buf);
    return 0;
}

char reader_peek(Reader *r) {
    return *r->pos;
}

char reader_peek_n(Reader *r, int n) {
    return *(r->pos + n - 1);
}

char reader_consume(Reader *r) {
    return *(r->pos++);
}

void reader_free(Reader *r) {
    free(r->start);
    r->pos = 0;
    r->start = 0;
}

char *reader_read_line(Reader *r) {
    char *start = r->pos;
    while (*r->pos != '\n' && *r->pos != 0) { r->pos++; }
    if (*r->pos != 0) {
        *r->pos = 0;
        r->pos++; //for the '\n'
    }
    return start;
}

size_t reader_bytes_left(Reader *r) {
    return r->len - (r->pos - r->start);
}

int reader_consume_if(Reader *r, char c) {
    if (reader_peek(r) == c) {
        reader_consume(r);
        return 1;
    }
    return 0;   
}
