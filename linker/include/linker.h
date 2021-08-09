#ifndef __LINKER_H
#define __LINKER_H
#include <stdint.h>
#include <stdlib.h>

struct SymTable {
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

size_t linker_link(uint8_t **input, size_t n, size_t *inp_lens, uint8_t **data);

#endif