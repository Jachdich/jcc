#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "../include/reader.h"
typedef struct Args Args;
typedef struct Instr Instr;
typedef struct Arg Arg;
struct Arg {
    int i;
    char *s;
};

/*struct Args {
    uint8_t a1;
    uint8_t a2;
    uint8_t a3;
    char *ca1;
    char *ca2;
    char *ca3;
    uint32_t movl_arg;
};*/

struct Instr {
    uint8_t opcode;
    Arg *args;
    char *label_at;
};

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
            printf("REG: %d\n", posargs[i].i);
        } else if (is_intlit(str)) {
            posargs[i].i = 0;
            while (*str >= '0' && *str <= '9') {
                posargs[i].i = posargs[i].i * 10 + *str++ - '0';
            }
            printf("INTLIT: %d\n", posargs[i].i);
        } else {
            size_t sz = 0;
            while (!issep(*str) && *str != 0) { str++; sz++; }
            char *ident = malloc(sz + 1);
            strcpy(ident, str - sz);
            ident[sz] = 0;
            posargs[i].s = ident;
            printf("IDENT: %s\n", ident);
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

int main() {
    Reader r;
    reader_construct(&r, "test.asm");

    Instr *instrs = malloc(sizeof(Instr) * 64);
    size_t len = 0;
    size_t cap = 64;
    char *next_lab = NULL;
    while (reader_bytes_left(&r) > 0) {
        char *line = reader_read_line(&r);
        while ((*line == '\t' || *line == ' ') && *line != 0) line++;
        if (*line == 0) continue;
        char *lab = read_label(&line);
        if (lab != NULL) {
            printf("New label: %s\n", lab);
        }
        if (*line == 0) { next_lab = lab; continue; }
        uint8_t opcode = get_opcode(&line);
        //printf("%s\n", line);
        Arg *args = read_args(line);
        if (len + 1 >= cap) {
            instrs = realloc(instrs, cap *= 2);
        }
        if (lab != NULL && next_lab != NULL) {
            printf("Error: two labels pointing to the same line! (fix me please this is a stupid restriction)\n");
            exit(0);
        }
        if (lab == NULL && next_lab != NULL) {
            lab = next_lab;
            next_lab = NULL;
        }
        instrs[len++] = (Instr){opcode, args, lab};
        //printf("%s\n", line);
    }

    char **syms = malloc(sizeof(char*) * len);
    int *locs = malloc(sizeof(size_t) * len);
    size_t spos = 0;
    int curr_pos = 0;
    for (size_t i = 0; i < len; i++) {
        Instr in = instrs[i];
        if (in.label_at != NULL) {
            printf("Defining sym '%s' as '%d'\n",in.label_at, curr_pos);
            syms[spos] = in.label_at;
            locs[spos] = curr_pos;
            spos += 1;
        }
        curr_pos += 1;
        if (in.opcode == 0x0f) {
            curr_pos += 1;
        }
    }

    curr_pos = 0;
    for (size_t i = 0; i < len; i++) {
        Instr *in = instrs +i;
        for (size_t ap = 0; ap <3; ap++) {
            if (in->args[ap].s != NULL) {
                for (size_t sp = 0; sp < spos; sp++) {
                    if (strcmp(in->args[ap].s, syms[sp]) == 0) {
                        free(in->args[ap].s);
                        in->args[ap].s = NULL;
                        in->args[ap].i = locs[sp] - curr_pos; //TODO make relative
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
    uint8_t *code = malloc(curr_pos * 4);
    size_t pos = 0;
    for (size_t i = 0; i < len; i++) {
        Instr in = instrs[i];
        Arg *a = in.args;
        switch (in.opcode) {
            case 0x00: code[pos++] = in.args[0].i; code[pos++] = in.args[0].i; code[pos++] = 0; code[pos++] = 0; break;
            case 0x01: code[pos++] = in.args[1]
        }
        printf("%d: %d %d %d\n", in.opcode, in.args[0].i, in.args[1].i, in.args[2].i);
        free(in.args);
    }
    free(instrs);

    

/*
    
    code[len++] = opcode;
    code[len++] = args.a1;
    code[len++] = args.a2;
    code[len++] = args.a3;

    if (opcode == 0x0f) {
        //movl takes another arg, for some goddamn reason
        *((uint32_t*)(code + len)) = args.movl_arg;
        len += 4;
        
    }*/
    reader_free(&r);
    return 0;
}
