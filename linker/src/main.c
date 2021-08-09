#include "../include/linker.h"
#include "../include/args.h"

#include <stdio.h>

size_t read_file(char *fname, uint8_t **ptr) {
    FILE *fp = fopen(fname, "r");
    if (fp == NULL) {
        return 1;
    }
    fseek(fp, 0, SEEK_END);
    size_t fsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    uint8_t *contents = malloc(fsize + 1);
    fread(contents, 1, fsize, fp);
    fclose(fp);
    *ptr = contents;
    return fsize;
}

int main(int argc, char **argv) {
    Args args = parse_args(argc, argv);
    if (args.status != 0) {
        return args.status;
    }

    uint8_t **files = malloc(sizeof(uint8_t*) * args.n_ifs);
    size_t *file_lens = malloc(sizeof(size_t) * args.n_ifs);
    uint8_t *code;

    for (int i = 0; i < args.n_ifs; i++) {
        file_lens[i] = read_file(args.ifnames[i], files + i);
    }
    
    size_t sz = linker_link(files, args.n_ifs, file_lens, &code);
    
    FILE *fp = fopen(args.ofname, "wb");
    if (fp == NULL) {
        fprintf(stderr, "Error opening output file '%s'\n", args.ofname);
    }
    fwrite(code, 1, sz, fp);
    fclose(fp);
    free(code);
    for (int i = 0; i < args.n_ifs; i++) {
        free(files[i]);
    }
    free(files);
    free(file_lens);
    free(args.ifnames);
    return 0;
}