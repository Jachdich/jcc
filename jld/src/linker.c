#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/linker.h"
#include <jtools/jobj.h>

int linker_link(uint8_t **input, size_t n, size_t *inp_lens, uint8_t **data) {
    uint8_t *total_code = malloc(3 * 4);
    struct LinkTable total = {NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0};
    table_init(&total);
    size_t total_codesz = 3 * 4;

    uint32_t max_last_id = 0;
    
    for (size_t i = 0; i < n; i++) {
        uint8_t *contents = input[i];
        printf("%lu %lu %lu\n", i, (size_t)contents, n);
        size_t len = inp_lens[i];

        struct LinkTable table;
        table_init(&table);
        uint8_t *code = NULL;
        struct ObjHeader header;
        size_t codelen = jobj_process_file(contents, len, &header, &code, &table);
        if (header.obj_type != OBJTYPE_LINKABLE) {
            table_free(&total);
            table_free(&table);
            free(total_code);
            free(code);
            return -1;
        }
        //printf("codelen init %lu\n", len);

        uint32_t delta = max_last_id;

        for (uint32_t i = 0; i < table.unres_cap; i++) {
            table.ids[i] += delta;
        }

        for (uint32_t i = 0; i < table.phoff_cap; i++) {
            uint32_t val;
            memcpy(&val, code + table.phoff[i], 4);
            //printf("Sym originally: %d\n", val);
            val += delta;
            //printf("Sym: %d\n", val);
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
        //printf("Codelen add: %lu\n", codelen);

        table_merge(&total, &table);
        //table_free(&table);
    }

    //printf("\n\n\nResolved:\n");
    //for (uint32_t i = 0; i < total.res_cap; i++) {
    //    printf("%s: %d\n", total.res_syms[i], total.locs[i]);
    //}
    
    //printf("\nUnresolved:\n");
    //for (uint32_t i = 0; i < total.unres_cap; i++) {
    //    printf("%s: %d\n", total.unres_syms[i], total.ids[i]);
    //}

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
            return INVALID_SYM_ID;
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
            return UNRESOLVED_SYMBOL;
        }
        val = replace_with;
        memcpy(total_code + total.phoff[i], &val, 4);
    }

    int32_t start_addr = -1;
    for (size_t i = 0; i < total.res_cap; i++) {
        if (strcmp("_start", total.res_syms[i]) == 0) {
            start_addr = total.locs[i];
        }
    }
    if (start_addr == -1) {
        return START_NOT_FOUND;
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
