#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "../include/reader.h"
#include "../include/args.h"
#include "../include/assembler.h"

int iswhite(char c) {
    return c == ' ' || c == '\t';
}

int issep(char c) {
    return iswhite(c) || c == ',';
}

int isnum(char c) {
    return c <= '9' && c >= '0';
}

int is_intlit(char *str) {
    if (!isnum(*str)) return 0;
    while (!issep(*str) && *str != 0) {
        if (*str < '0' || *str > '9') return 0;
        str++;
    }
    return 1;
}

int is_reg(char *str) {
    if (*str != 'r') return 0;
    str++;
    return is_intlit(str);
}

int is_twobyte(uint8_t opcode) {
    return opcode == 0x02 || opcode == 0x03 || opcode == 0x0f || opcode == 0x11 || opcode == 0x13 || opcode == 0x14;
}

uint8_t get_opcode(char **str) {
    char *init = *str;
    size_t sz = 0;
    while (!issep(**str) && **str != 0) {
        (*str)++;
        sz++;
    }
    //**str = 0;
    //(*str)++;
    while (issep(**str)) (*str)++;
    //printf("%s\n", init);
    if (strncmp(init, "mov", sz)  == 0) return 0x00;
    if (strncmp(init, "movi", sz) == 0) return 0x01;
    if (strncmp(init, "jc", sz)   == 0) return 0x02;
    if (strncmp(init, "jp", sz)   == 0) return 0x03;
    if (strncmp(init, "out", sz)  == 0) return 0x04;
    if (strncmp(init, "add", sz)  == 0) return 0x05;
    if (strncmp(init, "addi", sz) == 0) return 0x06;
    if (strncmp(init, "sub", sz)  == 0) return 0x07;
    if (strncmp(init, "subi", sz) == 0) return 0x08;
    if (strncmp(init, "mul", sz)  == 0) return 0x09;
    if (strncmp(init, "muli", sz) == 0) return 0x0a;
    if (strncmp(init, "div", sz)  == 0) return 0x0b;
    if (strncmp(init, "divi", sz) == 0) return 0x0c;
    if (strncmp(init, "mod", sz)  == 0) return 0x0d;
    if (strncmp(init, "modi", sz) == 0) return 0x0e;
    if (strncmp(init, "movl", sz) == 0) return 0x0f;
    if (strncmp(init, "halt", sz) == 0) return 0x10;
    if (strncmp(init, "call", sz) == 0) return 0x11;
    if (strncmp(init, "ret", sz)  == 0) return 0x12;
    if (strncmp(init, "movra", sz) == 0) return 0x13;
    if (strncmp(init, "movar", sz) == 0) return 0x14;
    printf("Unknown opcode %s\n", init);
    return -1;
}

char *read_label(char **str) {
    char *init = *str;
    while (*init != 0 && *init != ':') (init)++;
    if (*init == ':') {
        *init = 0;
        init++;
        while (iswhite(*init)) init++;
        char *label = *str;
        *str = init;
        return label;
    } else {
        return NULL;
    }
}

//compare two strings until one of them encounters a "seperator" character
int strsepcmp(char *a, char *b) {
    while (!issep(*a) && !issep(*b) && *b != 0 && *a != 0) {
        if (*a != *b) {
            return 1;
        }
        a++;
        b++;
    }
    if ((issep(*a) || *a == 0) != (issep(*b) || *b == 0)) {
        return 1;
    }
    return 0;
}

const char *argtypetostr(ArgType ty) {
    switch (ty) {
        case AT_NONE:  return "no argument";
        case AT_REG:   return "register";
        case AT_INT:   return "integer";
        case AT_LABEL: return "label";
        case AT_COND:  return "condition";
        default: return "undefined";
    }
}


Arg *read_args(char *str) {
    Arg *posargs = malloc(sizeof(Arg) * 3);
    posargs[0] = (Arg){0, NULL, AT_NONE};
    posargs[1] = (Arg){0, NULL, AT_NONE};
    posargs[2] = (Arg){0, NULL, AT_NONE};
    if (*str == 0) { return posargs; }
    for (int i = 0; i < 3; i++) {
        //printf("It's a str: %s\n", str);
        if (strsepcmp(str, "z") == 0) {
            str += 1;
            posargs[i].i = 1;
            posargs[i].t = AT_COND;
        } else if (strsepcmp(str, "nz") == 0) {
            str += 2;
            posargs[i].i = 3;
            posargs[i].t = AT_COND;
        } else if (strsepcmp(str, "c") == 0) {
            str += 1;
            posargs[i].i = 0;
            posargs[i].t = AT_COND;
        } else if (strsepcmp(str, "nc") == 0) {
            str += 2;
            posargs[i].i = 2;
            posargs[i].t = AT_COND;
        } else if (is_reg(str)) {
            str++;
            posargs[i].i = 0;
            while (*str >= '0' && *str <= '9') {
                posargs[i].i = posargs[i].i * 10 + *str++ - '0';
            }
            posargs[i].t = AT_REG;
        } else if (is_intlit(str)) {
            posargs[i].i = 0;
            while (*str >= '0' && *str <= '9') {
                posargs[i].i = posargs[i].i * 10 + *str++ - '0';
            }
            posargs[i].t = AT_INT;
        } else {
            size_t sz = 0;
            while (!issep(*str) && *str != 0) { str++; sz++; }
            if (sz != 0) {
                char *ident = malloc(sz + 1);
                strncpy(ident, str - sz, sz);
                ident[sz] = 0;
                posargs[i].s = ident;
                posargs[i].t = AT_LABEL;
            }
        }
        
        while (iswhite(*str)) { str++; }
        if (*str == ',') {
            str++;
        } else if (*str == '-') {
            str++;
            if (*str == '>') {
                str++; 
            } else {
                printf("Error: expected ->, got something else: %s\n", str - 1);
                exit(0);
            }
        } else if (*str == 0) {
            break;
        } else {
            printf("weird character '%s'\n", str);
            exit(0);
        }
        while (iswhite(*str)) { str++; }
    }
    return posargs;
}

void writeshort(uint8_t *code, size_t *pos, int n) {
    //*((short*)(code + ((*pos) += 2))) = n & 0xFFFF;
    code[(*pos)++] = n & 0xFF;
    code[(*pos)++] = (n >> 8) & 0xFF;
}

void writeqword(uint8_t *code, size_t *pos, int n) {
    //*((short*)(code + ((*pos) += 2))) = n & 0xFFFF;
    code[(*pos)++] = n & 0xFF;
    code[(*pos)++] = (n >> 8) & 0xFF;
    code[(*pos)++] = (n >> 16) & 0xFF;
    code[(*pos)++] = (n >> 24) & 0xFF;
}

int args_assert(Arg *args, ArgType a, ArgType b, ArgType c, int linenum, char *line) {
    ArgType tys[3] = {a, b, c};
    for (int i = 0; i < 3; i++) {
        if (args[i].t != tys[i]) {
            if (args[i].t == AT_INT && tys[i] == AT_LABEL) {
                fprintf(stderr, "%d: %s\nWarning: integer constant used when label was expected\n\n", linenum, line);
            } else if (tys[i] == AT_INT && tys[i] == AT_LABEL) {
                fprintf(stderr, "%d: %s\nWarning: label used when integer constant was expected\n\n", linenum, line);
            } else {
                fprintf(stderr, "%d: %s\nError: Expected %s, got %s instead\n",
                     linenum, line, argtypetostr(tys[i]), argtypetostr(args[i].t));
                return 0;
            }
        }
    }
    return 1;
}

int args_match(uint8_t opcode, Arg *args, int linenum, char *line) {
    switch (opcode) {
        case 0x00: return args_assert(args, AT_REG,   AT_REG,   AT_NONE, linenum, line);
        case 0x01: return args_assert(args, AT_INT,   AT_REG,   AT_NONE, linenum, line);
        case 0x02: return args_assert(args, AT_COND,  AT_LABEL, AT_REG,  linenum, line);
        case 0x03: return args_assert(args, AT_LABEL, AT_NONE,  AT_NONE, linenum, line);
        case 0x04: return args_assert(args, AT_REG,   AT_NONE,  AT_NONE, linenum, line);
        case 0x05: return args_assert(args, AT_REG,   AT_REG,   AT_REG,  linenum, line);
        case 0x06: return args_assert(args, AT_REG,   AT_INT,   AT_NONE, linenum, line);
        case 0x07: return args_assert(args, AT_REG,   AT_REG,   AT_REG,  linenum, line);
        case 0x08: return args_assert(args, AT_REG,   AT_INT,   AT_NONE, linenum, line);
        case 0x09: return args_assert(args, AT_REG,   AT_REG,   AT_REG,  linenum, line);
        case 0x0A: return args_assert(args, AT_REG,   AT_INT,   AT_NONE, linenum, line);
        case 0x0B: return args_assert(args, AT_REG,   AT_REG,   AT_REG,  linenum, line);
        case 0x0C: return args_assert(args, AT_REG,   AT_INT,   AT_NONE, linenum, line);
        case 0x0D: return args_assert(args, AT_REG,   AT_REG,   AT_REG,  linenum, line);
        case 0x0E: return args_assert(args, AT_REG,   AT_INT,   AT_NONE, linenum, line);
        case 0x0F: return args_assert(args, AT_INT,   AT_REG,   AT_NONE, linenum, line);
        case 0x10: return args_assert(args, AT_NONE,  AT_NONE,  AT_NONE, linenum, line);
        case 0x11: return args_assert(args, AT_LABEL, AT_NONE,  AT_NONE, linenum, line);
        case 0x12: return args_assert(args, AT_NONE,  AT_NONE,  AT_NONE, linenum, line);
        case 0x13: return args_assert(args, AT_REG,   AT_LABEL, AT_NONE, linenum, line);
        case 0x14: return args_assert(args, AT_LABEL,  AT_REG,  AT_NONE, linenum, line);
        default:
            fprintf(stderr, "Bug: Unrecognised opcode %02x\n", opcode);
            return 0;
    }
}

size_t gen_instrs(Reader *r, Instr **instrs_ptr) {
    *instrs_ptr = malloc(sizeof(Instr) * 64);
    Instr *instrs = *instrs_ptr;
    int line_num = 0;
    size_t len = 0;
    size_t cap = 64;
    char *next_lab = NULL;
    while (reader_bytes_left(r) > 0) {
        char *line = reader_read_line(r);
        char *line_init = malloc(strlen(line) + 1);
        strcpy(line_init, line);
        while (iswhite(*line) && *line != 0) line++;
        
        if (*line == 0) continue;
        char *lab = read_label(&line);

        if (*line == 0) { next_lab = lab; continue; }

        uint8_t opcode;
        Arg *args;
        int is_lit;
        if (strncmp(line, "dd ", 3) == 0) {
            uint32_t val = 0;
            line += 3;
            while (isnum(*line)) {
                val = val * 10 + *line++ - '0';
            }
            opcode = val & 0xFF;
            args = malloc(sizeof(Arg) * 3);
            args[0] = (Arg){(val  >> 8) & 0xFF, NULL, AT_INT};
            args[1] = (Arg){(val >> 16) & 0xFF, NULL, AT_INT};
            args[2] = (Arg){(val >> 24) & 0xFF, NULL, AT_INT};
            is_lit = 1;
        } else {
            opcode = get_opcode(&line);
            args = read_args(line);
            if (!args_match(opcode, args, line_num, line_init)) {
                exit(0);
            }
            is_lit = 0;
        }
        
        if (lab != NULL && next_lab != NULL) {
            printf("Error: two labels pointing to the same line! (fix me please this is a stupid restriction)\n");
            exit(0);
        }
        
        if (lab == NULL && next_lab != NULL) {
            lab = next_lab;
            next_lab = NULL;
        }
                
        instrs[len++] = (Instr){opcode, args, lab, is_lit};

        if (len >= cap) {
            *instrs_ptr = realloc(instrs, sizeof(Instr) * (cap *= 2));
            instrs = *instrs_ptr;
        }
        line_num++;
        free(line_init);
    }

    return len;
}

size_t resolve_symbols(Instr *instrs, size_t num_instrs, struct LinkTable *table) {
    table->res_syms = malloc(sizeof(char*) * 64);
    table->unres_syms = malloc(sizeof(char*) * 64);
    table->locs = malloc(sizeof(size_t) * 64);
    table->ids = malloc(sizeof(size_t) * 64);
    table->res_pos = 0;
    table->unres_pos = 0;
    table->res_cap = 64;
    table->unres_cap = 64;
    table->curr_id = 1;

    table->phoff = malloc(sizeof(uint32_t) * 64);
    table->phoff_pos = 0;
    table->phoff_cap = 64;
    
    int curr_pos = 0;
    for (size_t i = 0; i < num_instrs; i++) {
        Instr in = instrs[i];
        if (in.label_at != NULL) {
            table->res_syms[table->res_pos] = in.label_at;
            in.label_at = NULL;
            table->locs[table->res_pos] = curr_pos;
            table->res_pos += 1;
            if (table->res_pos >= table->res_cap) {
                table->res_syms = realloc(table->res_syms, sizeof(char*) * table->res_cap * 2);
                table->locs = realloc(table->locs, sizeof(size_t) * table->res_cap * 2);
                table->res_cap *= 2;
            }
        }
        
        curr_pos += 4;
        if (is_twobyte(in.opcode)) {
            curr_pos += 4;
        }
    }
    
    curr_pos = 0;
    for (size_t i = 0; i < num_instrs; i++) {
        Instr *in = instrs + i;
        
        for (size_t ap = 0; ap <3; ap++) {
            if (in->args[ap].s != NULL) {
                for (size_t sp = 0; sp < table->unres_pos; sp++) {
                    if (strcmp(in->args[ap].s, table->unres_syms[sp]) == 0) {
                        free(in->args[ap].s);
                        in->args[ap].s = NULL;
                        in->args[ap].i = table->ids[sp];
                        
                        break;
                    }
                }
                if (in->args[ap].s != NULL) {
                    table->unres_syms[table->unres_pos] = in->args[ap].s;
                    table->ids[table->unres_pos] = table->curr_id;
                    in->args[ap].i = table->curr_id;
                    in->args[ap].s = NULL;
                    table->unres_pos++;
                    table->curr_id++;
                    if (table->unres_pos >= table->unres_cap) {
                        table->unres_syms = realloc(table->unres_syms, sizeof(char*) * table->unres_cap * 2);
                        table->ids = realloc(table->ids, sizeof(size_t) * table->unres_cap * 2);
                        table->unres_cap *= 2;
                    }
                }

                table->phoff[table->phoff_pos++] = curr_pos + 4;

                if (table->phoff_pos >= table->phoff_cap) {
                    table->phoff = realloc(table->phoff, sizeof(uint32_t) * (table->phoff_cap *= 2));
                }
            }
        }
        curr_pos += 4;
        if (is_twobyte(in->opcode)) {
            curr_pos += 4;
        }
    }
    /*
    for (size_t i = 0; i < spos; i++) {
        free(syms[spos]);
    }
    free(syms);*/
    return curr_pos;
}

size_t reorder_args(uint8_t **code_ptr, Instr *instrs, size_t num_instrs, size_t num_words) {
    size_t codesize = num_words * 4;
    *code_ptr = malloc(codesize);
    uint8_t *code = *code_ptr;
    size_t pos = 0;
    for (size_t i = 0; i < num_instrs; i++) {
        Instr in = instrs[i];
        Arg *a = in.args;
        code[pos++] = in.opcode;
        if (in.is_lit) {
            code[pos++] = a[0].i;
            code[pos++] = a[1].i;
            code[pos++] = a[2].i;

            printf("LIT: %02x %02x %02x %02x\n", in.opcode, a[0].i, a[1].i, a[2].i);
        } else {
            switch (in.opcode) {
                case 0x00: code[pos++] = a[0].i; code[pos++] = a[1].i; code[pos++] = 0;     break;
                case 0x01: code[pos++] = a[1].i; writeshort(code, &pos, a[0].i);            break;
                case 0x02: code[pos++] = a[0].i; code[pos++] = a[2].i; code[pos++] = 0; writeqword(code, &pos, a[1].i); break;
                case 0x03: code[pos++] = 0; code[pos++] = 0; code[pos++] = 0; writeqword(code, &pos, a[0].i); break;
                case 0x04: code[pos++] = a[0].i; code[pos++] = 0; code[pos++] = 0;          break;
                
                case 0x05: code[pos++] = a[0].i; code[pos++] = a[1].i; code[pos++] = a[2].i;break;
                case 0x06: code[pos++] = a[0].i; writeshort(code, &pos, a[1].i);            break;
                case 0x07: code[pos++] = a[0].i; code[pos++] = a[1].i; code[pos++] = a[2].i;break;
                case 0x08: code[pos++] = a[0].i; writeshort(code, &pos, a[1].i);            break;
                case 0x09: code[pos++] = a[0].i; code[pos++] = a[1].i; code[pos++] = a[2].i;break;
                case 0x0a: code[pos++] = a[0].i; writeshort(code, &pos, a[1].i);            break;
                case 0x0b: code[pos++] = a[0].i; code[pos++] = a[1].i; code[pos++] = a[2].i;break;
                case 0x0c: code[pos++] = a[0].i; writeshort(code, &pos, a[1].i);            break;
                case 0x0d: code[pos++] = a[0].i; code[pos++] = a[1].i; code[pos++] = a[2].i;break;
                case 0x0e: code[pos++] = a[0].i; writeshort(code, &pos, a[1].i);            break;

                case 0x0f: code[pos++] = a[1].i; code[pos++] = 0; code[pos++] = 0; writeqword(code, &pos, a[0].i); break;
                case 0x10: code[pos++] = 0; code[pos++] = 0; code[pos++] = 0; break;
                case 0x11: code[pos++] = 0; code[pos++] = 0; code[pos++] = 0; writeqword(code, &pos, a[0].i); break;
                case 0x12: code[pos++] = 0; code[pos++] = 0; code[pos++] = 0; break;
                case 0x13: code[pos++] = a[0].i; code[pos++] = 0; code[pos++] = 0; writeqword(code, &pos, a[1].i); break;
                case 0x14: code[pos++] = a[1].i; code[pos++] = 0; code[pos++] = 0; writeqword(code, &pos, a[0].i); break;
                default: break;
            }
        }
        free(in.args);
    }
    return codesize;
}

/*

object file

Header:
    4: magic 'Lmao'
    4: len(unres_sym)
    4: len(res_sym)

n * unres_table:
    4: strlen(sym)
    len: sym
    4: id

n * res_table:
    4: strlen(sym)
    len: sym
    4: offset 

to_replace:
    4: num
    num * 4: offsets
*/

size_t assemble(Reader *src, uint8_t **out) {
    Instr *instrs;
    size_t num_instrs = gen_instrs(src, &instrs);
    struct LinkTable table;
    size_t num_words = resolve_symbols(instrs, num_instrs, &table) / 4;
    uint8_t *assembled_instructions;
    size_t codesize = reorder_args(&assembled_instructions, instrs, num_instrs, num_words);

    struct ObjHeader header = {{0xBA, 'L', 'L', 'S'}, table.unres_pos, table.res_pos};
    
    size_t unres_size = sizeof(size_t) * 2 * table.unres_pos;
    for (size_t i = 0; i < table.unres_pos; i++) {
        unres_size += strlen(table.unres_syms[i]);
    }

    size_t res_size = sizeof(size_t) * 2 * table.res_pos;
    for (size_t i = 0; i < table.res_pos; i++) {
        res_size += strlen(table.res_syms[i]);
    }
    
    uint8_t *data = malloc(sizeof(header) + codesize + unres_size + res_size + table.phoff_pos * sizeof(uint32_t) + sizeof(uint32_t));
    *out = data;

    //header
    memcpy(data, &header, sizeof(header));
    data += sizeof(header);

    //unres table
    data = write_table(data, table.unres_syms, table.ids, table.unres_pos);
    //res table
    data = write_table(data, table.res_syms, table.locs, table.res_pos);

    //placeholders table
    memcpy(data, &table.phoff_pos, sizeof(uint32_t));
    data += sizeof(uint32_t);
    memcpy(data, table.phoff, table.phoff_pos * sizeof(uint32_t));
    data += table.phoff_pos * sizeof(uint32_t);
    memcpy(data, assembled_instructions, codesize);
    data += codesize;

    table_free(&table);
    
    free(instrs);
    return data - *out;
}
