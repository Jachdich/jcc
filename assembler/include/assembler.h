#include <stdint.h>
#include "../include/reader.h"

typedef struct Instr Instr;
typedef struct Arg Arg;
typedef struct SymTable SymTable;

struct Arg {
    int i;
    char *s;
};

struct Instr {
    uint8_t opcode;
    Arg *args;
    char *label_at;
    int is_lit;
};

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

uint8_t get_opcode(char **str);
size_t gen_instrs(Reader *r, Instr **instrs_ptr);
size_t resolve_symbols(Instr *instrs, size_t num_instrs, SymTable *table);
size_t reorder_args(uint8_t **code_ptr, Instr *instrs, size_t num_instrs, size_t num_words);
size_t assemble(Reader *src, uint8_t **out);
