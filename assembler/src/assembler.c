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

int is_intlit(char *str) {
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

uint8_t get_opcode(char **str) {
    char *init = *str;
    while (**str != ' ' && **str != '\t' && **str != 0) {
        (*str)++;
    }
    **str = 0;
    (*str)++;
    //printf("%s\n", init);
    if (strcmp(init, "mov")  == 0) return 0;
    if (strcmp(init, "movi") == 0) return 1;
    if (strcmp(init, "jcr")  == 0) return 2;
    if (strcmp(init, "jr")   == 0) return 3;
    if (strcmp(init, "out")  == 0) return 4;
    if (strcmp(init, "add")  == 0) return 5;
    if (strcmp(init, "addi") == 0) return 6;
    if (strcmp(init, "sub")  == 0) return 7;
    if (strcmp(init, "subi") == 0) return 8;
    if (strcmp(init, "mul")  == 0) return 9;
    if (strcmp(init, "muli") == 0) return 10;
    if (strcmp(init, "div")  == 0) return 11;
    if (strcmp(init, "divi") == 0) return 12;
    if (strcmp(init, "mod")  == 0) return 13;
    if (strcmp(init, "modi") == 0) return 14;
    if (strcmp(init, "movl") == 0) return 15;
    if (strcmp(init, "halt") == 0) return 16;
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

Arg *read_args(char *str) {
    Arg *posargs = malloc(sizeof(Arg) * 3);
    posargs[0] = (Arg){0, NULL};
    posargs[1] = (Arg){0, NULL};
    posargs[2] = (Arg){0, NULL};
    for (int i = 0; i < 3; i++) {

        if (strncmp(str, "z", 1) == 0) {
            str += 1;
            posargs[i].i = 1;
        } else if (strncmp(str, "nz", 2) == 0) {
            str += 2;
            posargs[i].i = 3;
        } else if (strncmp(str, "c", 1) == 0) {
            str += 1;
            posargs[i].i = 0;
        } else if (strncmp(str, "nc", 2) == 0) {
            str += 2;
            posargs[i].i = 2;
        } else if (is_reg(str)) {
            str++;
            posargs[i].i = 0;
            while (*str >= '0' && *str <= '9') {
                posargs[i].i = posargs[i].i * 10 + *str++ - '0';
            }
        } else if (is_intlit(str)) {
            posargs[i].i = 0;
            while (*str >= '0' && *str <= '9') {
                posargs[i].i = posargs[i].i * 10 + *str++ - '0';
            }
        } else {
            size_t sz = 0;
            while (!issep(*str) && *str != 0) { str++; sz++; }
            char *ident = malloc(sz + 1);
            strcpy(ident, str - sz);
            ident[sz] = 0;
            posargs[i].s = ident;
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

size_t gen_instrs(Reader *r, Instr **instrs_ptr) {
    *instrs_ptr = malloc(sizeof(Instr) * 64);
    Instr *instrs = *instrs_ptr;
    size_t len = 0;
    size_t cap = 64;
    char *next_lab = NULL;
    while (reader_bytes_left(r) > 0) {
        char *line = reader_read_line(r);
        while (iswhite(*line) && *line != 0) line++;
        
        if (*line == 0) continue;
        char *lab = read_label(&line);

        if (*line == 0) { next_lab = lab; continue; }
        
        uint8_t opcode = get_opcode(&line);
        Arg *args = read_args(line);
        
        if (lab != NULL && next_lab != NULL) {
            printf("Error: two labels pointing to the same line! (fix me please this is a stupid restriction)\n");
            exit(0);
        }
        
        if (lab == NULL && next_lab != NULL) {
            lab = next_lab;
            next_lab = NULL;
        }
        
        instrs[len++] = (Instr){opcode, args, lab};

        if (len >= cap) {
            *instrs_ptr = realloc(instrs, sizeof(Instr) * (cap *= 2));
            instrs = *instrs_ptr;
        }
    }

    return len;
}

size_t resolve_symbols(Instr *instrs, size_t num_instrs) {
    char **syms = malloc(sizeof(char*) * 64);
    int *locs = malloc(sizeof(int) * 64);
    
    size_t spos = 0;
    size_t scap = 64;
    int curr_pos = 0;
    for (size_t i = 0; i < num_instrs; i++) {
        Instr in = instrs[i];
        if (in.label_at != NULL) {
            syms[spos] = in.label_at;
            locs[spos] = curr_pos;
            spos += 1;
            if (spos >= scap) {
                syms = realloc(syms, sizeof(char*) * scap * 2);
                locs = realloc(locs, sizeof(int) * scap * 2);
                scap *= 2;
            }
        }
        
        curr_pos += 1;
        if (in.opcode == 0x0f) {
            curr_pos += 1;
        }
    }

    curr_pos = 0;
    for (size_t i = 0; i < num_instrs; i++) {
        Instr *in = instrs + i;
        for (size_t ap = 0; ap <3; ap++) {
            if (in->args[ap].s != NULL) {
                for (size_t sp = 0; sp < spos; sp++) {
                    if (strcmp(in->args[ap].s, syms[sp]) == 0) {
                        free(in->args[ap].s);
                        in->args[ap].s = NULL;
                        in->args[ap].i = locs[sp] - curr_pos;
                        break;
                    }
                }
                if (in->args[ap].s != NULL) {
                    printf("Error: unknown label '%s'\n", in->args[ap].s);
                    exit(0);
                }
            }
        }
        curr_pos += 1;
        if (in->opcode == 0x0f) {
            curr_pos += 1;
        }
    }

    for (size_t i = 0; i < spos; i++) {
        free(syms[spos]);
    }
    free(syms);
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
        switch (in.opcode) {
            case 0x00: code[pos++] = a[0].i; code[pos++] = a[1].i; code[pos++] = 0;     break;
            case 0x01: code[pos++] = a[1].i; writeshort(code, &pos, a[0].i);            break;
            case 0x02: code[pos++] = a[0].i; writeshort(code, &pos, a[1].i);            break;
            case 0x03: code[pos++] = 0;      writeshort(code, &pos, a[0].i);            break;
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
            default: break;
        }
        free(in.args);
    }
    return codesize;
}

size_t assemble(Reader *src, uint8_t **out) {
    Instr *instrs;
    size_t num_instrs = gen_instrs(src, &instrs);
    size_t num_words = resolve_symbols(instrs, num_instrs);
    size_t codesize = reorder_args(out, instrs, num_instrs, num_words);
    free(instrs);
    return codesize;
}
