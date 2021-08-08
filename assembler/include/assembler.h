#include <stdint.h>
#include "../include/reader.h"

typedef struct Instr Instr;
typedef struct Arg Arg;
struct Arg {
    int i;
    char *s;
};

struct Instr {
    uint8_t opcode;
    Arg *args;
    char *label_at;
};

uint8_t get_opcode(char **str);
size_t gen_instrs(Reader *r, Instr **instrs_ptr);
size_t resolve_symbols(Instr *instrs, size_t num_instrs);
size_t reorder_args(uint8_t **code_ptr, Instr *instrs, size_t num_instrs, size_t num_words);
size_t assemble(Reader *src, uint8_t **out);
