#include "../include/args.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void usage(char *ename) {
    printf("Usage: %s [-h] [-o outfile] infile [infile 1 [infile 2...]]\n", ename);
    printf("\t-o\t--output <outfile>\tSpecify the file to output linked executable to. If not specified, assume a.out\n");
    printf("\t-h\t--help\t\t\tPrint this help message and exit\n");
}

Args parse_args(int argc, char **argv) {
    char **ifnames = malloc(sizeof(char*) * argc); //malloc max possible number, for simplicity
    memset(ifnames, 0, sizeof(char*) * argc);
    int n_ifs = 0;
    char *ofname = NULL;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-o") == 0 ||
            strcmp(argv[i], "--output") == 0) {
            i++;
            if (ofname != NULL) {
                fprintf(stderr, "Don't specify more than one outfile\n");
                usage(argv[0]);
                return (Args){1, NULL, NULL, 0};
            }
            if (i >= argc) {
                fprintf(stderr, "%s expects one argument: filename\n", argv[i - 1]);
                usage(argv[0]);
                return (Args){1, NULL, NULL, 0};
           }
            ofname = argv[i];
        } else if (strcmp(argv[i], "-h") == 0 ||
                   strcmp(argv[i], "--help") == 0) {
            usage(argv[0]);
            exit(0);
        } else {
            ifnames[n_ifs++] = argv[i];
        }
        
    }
    if (ifnames[0] == NULL) {
        fprintf(stderr, "Please specify at least one input file\n");
        usage(argv[0]);
        return (Args){1, NULL, NULL, 0};
    }
    return (Args){0, ifnames, ofname, n_ifs};
}
