#include <stdio.h>
#include <stdlib.h>
#include "../include/reader.h"
#include "../include/lexer.h"
#include "../include/preprocessor.h"
#include "../include/parser.h"

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
    Error lex_err = lex_read_tokens(&s, &preproc_r);
    if (lex_err.status_code != 0) {
        fprintf(stderr, "%s: %s\n", argv[0], lex_err.message);
        error_free(&lex_err);
        return lex_err.status_code;
    }
    error_free(&lex_err);
    
    Error preproc_err = preprocess_tokens(&s);
    if (preproc_err.status_code != 0) {
        fprintf(stderr, "%s: %s\n", argv[0], preproc_err.message);
        error_free(&preproc_err);
        return preproc_err.status_code;
    }
    error_free(&preproc_err);

    lex_print_tokens(&s);
    char *src = lex_reconstruct_src(&s);
    printf("%s\n", src);
    free(src);

    AST *ast = malloc(sizeof(AST));
    int ast_err = ast_gen(ast, &s);
    if (ast_err != 0) {
        fprintf(stderr, "%s: Parser returned non-zero status %d\n", argv[0], ast_err);
        return ast_err;
    }
    
    ast_print(ast);
    ast_free(ast);
    lex_free(&s);
    reader_free(&preproc_r);
    return 0;
}
