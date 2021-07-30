#include <stdio.h>
#include <stdlib.h>
#include "../include/reader.h"
#include "../include/lexer.h"
#include "../include/preprocessor.h"

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

    char *preprocessed = preprocess_includes(&r);
    reader_free(&r);
    if (preprocessed == NULL) {
        printf("Some kind of error happened lol\n");
        return 1;
    }

    Reader preproc_r;
    reader_construct_from(&preproc_r, preprocessed);

    LexTokenStream s;
    lex_init(&s);
    int lex_err = lex_read_tokens(&s, &preproc_r);
    if (lex_err != 0) {
        fprintf(stderr, "%s: Lexer returned non-zero status %d\n", argv[0], lex_err);
        return lex_err;
    }
    preprocess_tokens(&s);

    lex_print_tokens(&s);
    char *src = lex_reconstruct_src(&s);
    printf("%s\n", src);
    free(src);
    lex_free(&s);
    reader_free(&preproc_r);
    return 0;
}
