#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/reader.h"
#include "../include/lexer.h"
#include "../include/preprocessor.h"
#include "../include/parser.h"
#include "../include/codegen.h"
#include "../include/args.h"

int main(int argc, char **argv) {
    Args args = parse_args(argc, argv);
    if (args.status != 0) {
        return args.status;
    }

    Reader r;
    int err = reader_construct(&r, args.ifname);
    if (err != 0) {
        fprintf(stderr, "%s: cannot read file %s: no such file\n", argv[0], args.ifname);
        return err;
    }

    char *preprocessed = preprocess_includes(&r);
    //printf("%s\n\n\n", preprocessed);
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
        lex_free(&s);
        reader_free(&preproc_r);
        return lex_err.status_code;
    }
    error_free(&lex_err);
    
    Error preproc_err = preprocess_tokens(&s);
    if (preproc_err.status_code != 0) {
        fprintf(stderr, "%s: %s\n", argv[0], preproc_err.message);
        error_free(&preproc_err);
        lex_free(&s);
        reader_free(&preproc_r);
        return preproc_err.status_code;
    }
    error_free(&preproc_err);

    lex_print_tokens(&s);
    char *src = lex_reconstruct_src(&s);
    printf("%s\n", src);
    free(src);

    AST ast;
    SymTable table;
    int ast_err = ast_gen(&ast, &s, &table);
    if (ast_err != 0) {
        fprintf(stderr, "%s: Parser returned non-zero status %d\n", argv[0], ast_err);
        lex_free(&s);
        reader_free(&preproc_r);
        ast_free(&ast);
        return ast_err;
    }
    
    ast_print(&ast);
    char *code;
    cg_gen(&ast, &code, &table);

    if (args.ofname == NULL) {
        printf("%s", code);
    } else {
        FILE *fp = fopen(args.ofname, "w");
        if (fp == NULL) {
            fprintf(stderr, "Error opening output file '%s'\n", args.ofname);
        }
        fprintf(fp, "%s", code);
        fclose(fp);
    }
    
    free(code);
    ast_free(&ast);
    lex_free(&s);
    reader_free(&preproc_r);
    return 0;
}
