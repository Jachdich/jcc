#include <stdint.h>
#include <jtools/jobj.h>
#include <jtools/reader.h>

typedef struct Instr Instr;
typedef struct Arg Arg;
typedef enum ArgType ArgType;

enum ArgType {
    AT_NONE,
    AT_REG,
    AT_INT,
    AT_LABEL,
};

struct Arg {
    int i;
    char *s;
    ArgType t;
};

struct Instr {
    uint8_t opcode;
    Arg *args;
    char **labels_at;
    int num_labels_at;
    int is_lit;
};

uint8_t get_opcode(char **str);
size_t gen_instrs(Reader *r, Instr **instrs_ptr);
size_t resolve_symbols(Instr *instrs, size_t num_instrs, struct LinkTable *table);
size_t reorder_args(uint8_t **code_ptr, Instr *instrs, size_t num_instrs, size_t num_words);
size_t assemble(Reader *src, uint8_t **out);
