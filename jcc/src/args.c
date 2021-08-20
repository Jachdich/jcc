#include "../include/args.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void usage(char *ename) {
    printf("Usage: %s [-hg] [-o outfile] infile\n", ename);
    printf("\t-o\t--output <outfile>\tSpecify the file to output compiled code to. If not specified, stdout is assumed\n");
    printf("\t-g\t--debug\t\t\tEnable debug symbols in the assembly code\n");
    printf("\t-h\t--help\t\t\tPrint this help message and exit\n");
}

Args parse_args(int argc, char **argv) {
    char *ifname = NULL;
    char *ofname = NULL;
    Args invalid_args = (Args){1, 0, NULL, NULL};
    int debug = 0;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-o") == 0 ||
            strcmp(argv[i], "--output") == 0) {
            i++;
            if (ofname != NULL) {
                fprintf(stderr, "Don't specify more than one outfile\n");
                usage(argv[0]);
                return invalid_args;
            }
            if (i >= argc) {
                fprintf(stderr, "%s expects one argument: filename\n", argv[i - 1]);
                usage(argv[0]);
                return invalid_args;
           }
            ofname = argv[i];
        } else if (strcmp(argv[i], "-h") == 0 ||
                   strcmp(argv[i], "--help") == 0) {
            usage(argv[0]);
            exit(0);
        } else if (strcmp(argv[i], "-g") == 0 ||
                   strcmp(argv[i], "--debug") == 0) {
            debug = 1;
        } else {
            if (ifname != NULL) {
                fprintf(stderr, "Don't specify more than one infile\n");
                usage(argv[0]);
                return invalid_args;
            }
            ifname = argv[i];
        }
        
    }
    if (ifname == NULL) {
        fprintf(stderr, "Please specify exactly one input file\n");
        usage(argv[0]);
        return invalid_args;
    }
    return (Args){0, debug, ifname, ofname};
}
