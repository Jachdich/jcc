#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/reader.h"
#include "../include/lexer.h"
#include "../include/preprocessor.h"
#include "../include/parser.h"
#include "../include/codegen.h"

struct Args {
    int status;
    char *ifname;
    char *ofname;
};

typedef struct Args Args;

void usage(char *ename) {
    printf("Usage: %s [-h] [-o outfile] infile\n", ename);
    printf("\t-o\t--output <outfile>\tSpecify the file to output compiled code to. If not specified, stdout is assumed\n");
    printf("\t-h\t--help\t\t\tPrint this help message and exit\n");
}

Args parse_args(int argc, char **argv) {
    char *ifname = NULL;
    char *ofname = NULL;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-o") == 0 ||
            strcmp(argv[i], "--output") == 0) {
            i++;
            if (ofname != NULL) {
                fprintf(stderr, "Don't specify more than one outfile\n");
                usage(argv[0]);
                return (Args){1, NULL, NULL};
            }
            if (i >= argc) {
                fprintf(stderr, "%s expects one argument: filename\n", argv[i - 1]);
                usage(argv[0]);
                return (Args){1, NULL, NULL};
           }
            ofname = argv[i];
        } else if (strcmp(argv[i], "-h") == 0 ||
                   strcmp(argv[i], "--help") == 0) {
            usage(argv[0]);
            exit(0);
        } else {
            if (ifname != NULL) {
                fprintf(stderr, "Don't specify more than one infile\n");
                usage(argv[0]);
                return (Args){1, NULL, NULL};
            }
            ifname = argv[i];
        }
        
    }
    if (ifname == NULL) {
        fprintf(stderr, "Please specify exactly one input file\n");
        usage(argv[0]);
        return (Args){1, NULL, NULL};
    }
    return (Args){0, ifname, ofname};
}

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
    int ast_err = ast_gen(&ast, &s);
    if (ast_err != 0) {
        fprintf(stderr, "%s: Parser returned non-zero status %d\n", argv[0], ast_err);
        lex_free(&s);
        reader_free(&preproc_r);
        ast_free(&ast);
        return ast_err;
    }
    
    ast_print(&ast);
    char *code;
    cg_gen(&ast, &code);

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
