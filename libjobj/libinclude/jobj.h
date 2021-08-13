#ifndef __OBJ_H
#define __OBJ_H
#include <stdlib.h>
#include <stdint.h>

struct LinkTable {
    char **res_syms;
    char **unres_syms;
    uint32_t *locs;
    uint32_t *ids;

    uint32_t *phoff;
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

void table_merge(struct LinkTable *a, struct LinkTable *b);
void table_init(struct LinkTable *t);
void table_free(struct LinkTable *table);
size_t jobj_process_file(uint8_t *data, size_t fsize, uint8_t **code, struct LinkTable *table);
uint8_t *write_table(uint8_t *data, char **syms, uint32_t *ids, uint32_t num);
uint8_t  *read_table(uint8_t *data, char **syms, uint32_t *ids, uint32_t num);

#endif
