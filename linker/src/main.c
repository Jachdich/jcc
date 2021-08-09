#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct SymTable {
    char **res_syms;
    char **unres_syms;
    uint32_t *locs;
    uint32_t *ids;

    uint32_t *placeholder_offsets;
    uint32_t phoff_pos;
    uint32_t phoff_cap;
    
    size_t res_pos;
    size_t res_cap;
    
    size_t unres_pos;
    size_t unres_cap;
    size_t curr_id;
};

struct ObjHeader {
    char magic[4];
    uint32_t unres_sym_len;
    uint32_t res_sym_len;
};

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

uint8_t *read_table(uint8_t *data, char **syms, uint32_t *ids, uint32_t num) {
    for (uint32_t i = 0; i < num; i++) {
        uint32_t slen;
        memcpy(&slen, data, 4);
        data += 4;
        syms[i] = malloc(slen + 1);
        memcpy(syms[i], data, slen);
        syms[i][slen] = 0;
        data += slen;
        uint32_t value;
        memcpy(&value, data, 4);
        data += 4;
        ids[i] = value;
        printf("%s: %d\n", syms[i], ids[i]);
    }
    return data;
}

void process_file(uint8_t *data, size_t fsize) {
    uint8_t *orig_data = data;
    struct ObjHeader header;
    memcpy(&header, data, sizeof(header));
    data += sizeof(header);

    struct SymTable table;
    table.unres_syms = malloc(sizeof(char*) * header.unres_sym_len);
    table.ids = malloc(sizeof(uint32_t) * header.unres_sym_len);
    table.res_syms = malloc(sizeof(char*) * header.res_sym_len);
    table.locs = malloc(sizeof(uint32_t) * header.res_sym_len);
    printf("Unresolved:\n");
    data = read_table(data, table.unres_syms, table.ids, header.unres_sym_len);
    printf("Resolved:\n");
    data = read_table(data, table.res_syms, table.locs, header.res_sym_len);

    uint32_t num_to_replace;
    memcpy(&num_to_replace, data, 4);
    data += 4;
    uint32_t *to_replace = malloc(sizeof(uint32_t) * num_to_replace);
    memcpy(to_replace, data, sizeof(uint32_t) * num_to_replace);
    data += sizeof(uint32_t) * num_to_replace;
    printf("To replace:\n");
    for (uint32_t i = 0; i < num_to_replace; i++) {
        printf("%d\n", to_replace[i]);
    }
    
}

int main(int argc, char **argv) {
    uint8_t *contents;
    size_t len = read_file(argv[1], &contents);
    process_file(contents, len);
    return 0;
}