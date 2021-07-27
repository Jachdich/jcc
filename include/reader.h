#ifndef __READER_H
#define __READER_H
#include <stddef.h>
struct Reader {
    char *start;
    char *pos;
    size_t len;
};

typedef struct Reader Reader;

int reader_construct(Reader *r, char *fname);
int reader_construct_from(Reader *r, char *buf);
char reader_peek(Reader *r);
char reader_consume(Reader *r);
char *reader_read_line(Reader *r);
void reader_free(Reader *r);
size_t reader_bytes_left(Reader *r);

#endif