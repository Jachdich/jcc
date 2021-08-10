#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/linker.h"

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

size_t process_file(uint8_t *data, size_t fsize, uint8_t **code, struct SymTable *table) {
    uint8_t *orig_data = data;
    struct ObjHeader header;
    memcpy(&header, data, sizeof(header));
    data += sizeof(header);

    table->unres_syms = malloc(sizeof(char*) * header.unres_sym_len);
    table->ids = malloc(sizeof(uint32_t) * header.unres_sym_len);
    table->res_syms = malloc(sizeof(char*) * header.res_sym_len);
    table->locs = malloc(sizeof(uint32_t) * header.res_sym_len);
    table->res_cap = header.res_sym_len;
    table->res_pos = 0;
    table->unres_cap = header.unres_sym_len;
    table->unres_pos = 0;
    printf("Unresolved:\n");
    data = read_table(data, table->unres_syms, table->ids, header.unres_sym_len);
    printf("Resolved:\n");
    data = read_table(data, table->res_syms, table->locs, header.res_sym_len);

    uint32_t num_to_replace;
    memcpy(&num_to_replace, data, 4);
    data += 4;
    table->phoff = malloc(sizeof(uint32_t) * num_to_replace);
    memcpy(table->phoff, data, sizeof(uint32_t) * num_to_replace);
    data += sizeof(uint32_t) * num_to_replace;
    printf("To replace:\n");
    for (uint32_t i = 0; i < num_to_replace; i++) {
        printf("%d\n", table->phoff[i]);
    }
    table->phoff_cap = num_to_replace;
    table->phoff_pos = 0;

    size_t bytes_read = data - orig_data;
    size_t bytes_to_read = fsize - bytes_read;
    *code = malloc(bytes_to_read);
    memcpy(*code, data, bytes_to_read);
    return bytes_to_read;
}

void table_free(struct SymTable *table) {
    free(table->res_syms);
    free(table->unres_syms);
    free(table->locs);
    free(table->ids);
    free(table->phoff);
}

void table_merge(struct SymTable *a, struct SymTable *b) {
    a->unres_syms = realloc(a->unres_syms, a->unres_cap + b->unres_cap);
    a->res_syms = realloc(a->res_syms, a->res_cap + b->res_cap);
    a->ids = realloc(a->ids, a->unres_cap + b->unres_cap);
    a->locs = realloc(a->locs, a->res_cap + b->res_cap);

    for (uint32_t i = 0; i < b->unres_cap; i++) {
        a->unres_syms[i + a->unres_cap] = b->unres_syms[i];
        a->ids[i + a->unres_cap] = b->ids[i];
    }
    
    for (uint32_t i = 0; i < b->res_cap; i++) {
        a->res_syms[i + a->res_cap] = b->res_syms[i];
        a->locs[i + a->res_cap] = b->locs[i];
    }
    
    a->unres_cap += b->unres_cap;
    a->res_cap += b->res_cap;
    if (a->phoff_cap == 0) {
        a->phoff = NULL;
    }
    if (b->phoff_cap != 0) {
        a->phoff = realloc(a->phoff, a->phoff_cap + b->phoff_cap);
        for (uint32_t i = 0; i < b->phoff_cap; i++) {
            a->phoff[a->phoff_cap + i] = b->phoff[i];
        }
        a->phoff_cap += b->phoff_cap;
        printf("phoff: \n");
        for (uint8_t i = 0; i < a->phoff_cap; i++) {
            printf("%d, ", a->phoff[i]);
        }
        printf("\n");
    }
}

void table_init(struct SymTable *t) {
    t->unres_syms = NULL;
    t->res_syms = NULL;
    t->ids = NULL;
    t->locs = NULL;
    t->phoff = NULL;
    t->unres_cap = 0;
    t->unres_pos = 0;
    t->res_cap = 0;
    t->res_pos = 0;
    t->phoff_pos = 0;
    t->phoff_cap = 0;
    t->curr_id = 0;
}

size_t linker_link(uint8_t **input, size_t n, size_t *inp_lens, uint8_t **data) {
    uint8_t *total_code = malloc(3 * 4);
    struct SymTable total;
    table_init(&total);
    size_t total_codesz = 3 * 4;

    uint32_t max_last_id = 0;
    
    for (size_t i = 0; i < n; i++) {
        uint8_t *contents = input[i];
        size_t len = inp_lens[i];

        struct SymTable table;
        uint8_t *code;
        size_t codelen = process_file(contents, len, &code, &table);

        uint32_t delta = max_last_id;

        for (uint32_t i = 0; i < table.unres_cap; i++) {
            table.ids[i] += delta;
        }

        for (uint32_t i = 0; i < table.phoff_cap; i++) {
            uint32_t val;
            memcpy(&val, code + table.phoff[i], 4);
            val += delta;
            printf("Sym: %d\n", val);
            if (val > max_last_id) max_last_id = val;
            memcpy(code + table.phoff[i], &val, 4);
        }

        for (uint32_t i = 0; i < table.phoff_cap; i++) {
            table.phoff[i] += total_codesz;
        }

        for (uint32_t i = 0; i < table.res_cap; i++) {
            table.locs[i] += total_codesz;
        }
        
        total_code = realloc(total_code, total_codesz + codelen);
        memcpy(total_code + total_codesz, code, codelen);
        free(code);
        total_codesz += codelen;

        table_merge(&total, &table);
        table_free(&table);
    }

    printf("\n\n\nResolved:\n");
    for (uint32_t i = 0; i < total.res_cap; i++) {
        printf("%s: %d\n", total.res_syms[i], total.locs[i]);
    }
    
    printf("\nUnresolved:\n");
    for (uint32_t i = 0; i < total.unres_cap; i++) {
        printf("%s: %d\n", total.unres_syms[i], total.ids[i]);
    }

    for (uint32_t i = 0; i < total.phoff_cap; i++) {
        uint32_t val;
        memcpy(&val, total_code + total.phoff[i], 4);
        int32_t sym_index = -1;
        for (uint32_t j = 0; j < total.unres_cap; j++) {
            if (total.ids[j] == val) {
                sym_index = j;
                break;
            }
        }
        
        if (sym_index == -1) {
            printf("Some horrible error happened and the symbol id %d at offset %d is not in any unresolved symbol table\n", val, total.phoff[i]);
            for (uint32_t e = 0; e < total_codesz; e += 4) {
                printf("%02x %02x %02x %02x\n", total_code[e], total_code[e + 1], total_code[e + 2], total_code[e + 3]);
            }
            exit(0);
        }

        int32_t replace_with = -1;
        for (uint32_t j = 0; j < total.res_cap; j++) {
            if (strcmp(total.unres_syms[sym_index], total.res_syms[j]) == 0) {
                replace_with = total.locs[j];
                break;
            }
        }

        if (replace_with == -1) {
            printf("Unresolved symbol '%s' was not found\n", total.unres_syms[sym_index]);
            exit(0);
        }
        val = replace_with;
        memcpy(total_code + total.phoff[i], &val, 4);
    }

    uint32_t start_addr = -1;
    for (size_t i = 0; i < total.res_cap; i++) {
        if (strcmp("_start", total.res_syms[i]) == 0) {
            start_addr = total.locs[i];
        }
    }
    total_code[0] = 0x11;
    total_code[1] = 0x0;
    total_code[2] = 0x0;
    total_code[3] = 0x0;

    memcpy(total_code + 4, &start_addr, 4);

    total_code[8] = 0x10;
    total_code[9] = 0x0;
    total_code[10] = 0x0;
    total_code[11] = 0x0;

    table_free(&total);
    
    *data = total_code;
    return total_codesz + 3 * 4;
}
