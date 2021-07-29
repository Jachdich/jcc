#include "../include/preprocessor.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
/*struct Vector {
    void *
};

struct State {
    char *toRep;char
};

typedef struct State State;*/

void preprocess_tokens(LexTokenStream *input) {
    
}

char *preprocess_includes(Reader *input) {
    char *output = malloc(reader_bytes_left(input));
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
            output = realloc(output, curr_bytes + reader_bytes);
            ptr = output + delta;
            strncpy(ptr, r.start, reader_bytes);
            reader_free(&r);
            ptr += reader_bytes;
            
        } else {
            strcpy(ptr, line);
            ptr += strlen(line) + 1;
            *(ptr - 1) = '\n';
        }
    }
    *ptr = 0;
    return output;
}