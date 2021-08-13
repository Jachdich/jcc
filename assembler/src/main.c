#include "../include/assembler.h"
#include "../include/args.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    Args args = parse_args(argc, argv);
    if (args.status != 0) {
        return args.status;
    }
    Reader r;
    reader_construct(&r, args.ifname);

    uint8_t *bin_code;
    size_t codesize = assemble(&r, &bin_code);

    if (args.print_bytes) {
        for (size_t i = 0; i < codesize; i += 4) {
            printf("%02x %02x %02x %02x\n", bin_code[i], bin_code[i + 1], bin_code[i + 2], bin_code[i + 3]);
        }
    }

    if (args.ofname != NULL) {
        FILE *fp = fopen(args.ofname, "wb");
        if (fp == NULL) {
            fprintf(stderr, "Error opening output file '%s'\n", args.ofname);
        }
        fwrite(bin_code, 1, codesize, fp);
        fclose(fp);
    }
    
    free(bin_code);
    reader_free(&r);
    return 0;
}
