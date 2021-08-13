#include "../include/jobj.h"
#include <string.h>
#include <stdio.h>

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

uint8_t *write_table(uint8_t *data, char **syms, uint32_t *ids, uint32_t num) {
    for (size_t i = 0; i < num; i++) {
        uint32_t symlen = strlen(syms[i]);
        char *sym = syms[i];
        uint32_t id = ids[i];
        memcpy(data, &symlen, 4);
        data += 4;
        memcpy(data, sym, symlen);
        data += symlen;
        memcpy(data, &id, 4);
        data += 4;
    }
    return data;
}


size_t jobj_process_file(uint8_t *data, size_t fsize, uint8_t **code, struct LinkTable *table) {
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
    printf("To replace %d:\n", num_to_replace);
    for (uint32_t i = 0; i < num_to_replace; i++) {
        printf("%d\n", table->phoff[i]);
    }
    table->phoff_cap = num_to_replace;
    table->phoff_pos = 0;

    size_t bytes_read = data - orig_data;
    size_t bytes_to_read = fsize - bytes_read;
    printf("Bytes read: %lu\n", bytes_read);
    *code = malloc(bytes_to_read);
    memcpy(*code, data, bytes_to_read);
    return bytes_to_read;
}

void table_init(struct LinkTable *t) {
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

void table_free(struct LinkTable *table) {
    free(table->res_syms);
    free(table->unres_syms);
    free(table->locs);
    free(table->ids);
    free(table->phoff);
}

void table_merge(struct LinkTable *a, struct LinkTable *b) {
    a->unres_syms = realloc(a->unres_syms, (a->unres_cap + b->unres_cap) * sizeof(*a->unres_syms));
    a->res_syms = realloc(a->res_syms, (a->res_cap + b->res_cap) * sizeof(*a->res_syms));
    a->ids = realloc(a->ids, (a->unres_cap + b->unres_cap) * sizeof(*a->ids));
    a->locs = realloc(a->locs, (a->res_cap + b->res_cap) * sizeof(*a->locs));;
    printf("A unres size: %lu\nB unres size: %lu\n", a->unres_cap, b->unres_cap);
    printf("New unres size: %lu\n", a->unres_cap + b->unres_cap);

    for (uint32_t i = 0; i < b->unres_cap; i++) {
        printf("New unres index: %d %lu\n", i, i + a->unres_cap);
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
        a->phoff = realloc(a->phoff, (a->phoff_cap + b->phoff_cap) * sizeof(*a->phoff));
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

