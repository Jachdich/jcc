#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <jobj/jobj.h>

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("provide exactly one file please lol\n");
        return 1;
    }
    char *fname = argv[1];
    
    FILE *fp = fopen(fname, "r");
    if (fp == NULL) {
        printf("file does not exist xx\n");
        return 1;
    }
    fseek(fp, 0, SEEK_END);
    size_t fsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    uint8_t *contents = malloc(fsize);
    fread(contents, 1, fsize, fp);
    fclose(fp);

    struct LinkTable table;
    table_init(&table);
    uint8_t *code = NULL;
    size_t codelen = jobj_process_file(contents, fsize, &code, &table);

    printf("%s defines the following symbols\n", fname);
    for (size_t i = 0; i < table.res_cap; i++) {
        printf("0x%02x: %s\n", table.locs[i], table.res_syms[i]);
    }

    printf("\nand requires the definition of the following symbols\n");
    for (size_t i = 0; i < table.unres_cap; i++) {
        printf("%s (id %d)\n", table.unres_syms[i], table.ids[i]);
    }

    printf("\nThe following offsets contain IDs of required symbols\n");
    for (size_t i = 0; i < table.phoff_cap; i++) {
        printf("%02x\n", table.phoff[i]);
    }

    printf("\nBinary code defined in this file:\n");

    for (size_t i = 0; i < codelen; i += 4) {
        printf("%02x: %02x %02x %02x %02x\n", (int)i, code[i], code[i + 1], code[i + 2], code[i + 3]);
    }

    free(contents);

    return 0;
}
