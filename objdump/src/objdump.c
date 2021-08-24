#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <jobj/jobj.h>

const char *op_name(uint8_t op) {
    switch (op) {
        case 0x00: return "mov";
        case 0x01: return "movi";
        case 0x02: return "jz";
        case 0x03: return "jp";
        case 0x04: return "out";
        case 0x05: return "add";
        case 0x06: return "addi";
        case 0x07: return "sub";
        case 0x08: return "subi";
        case 0x09: return "mul";
        case 0x0a: return "muli";
        case 0x0b: return "div";
        case 0x0c: return "divi";
        case 0x0d: return "mod";
        case 0x0e: return "modi";
        case 0x0f: return "movl";
        case 0x10: return "halt";
        case 0x11: return "call";
        case 0x12: return "ret";
        case 0x13: return "movra";
        case 0x14: return "movar";
        case 0x15: return "jnz";
        case 0x16: return "cmp";
        case 0x17: return "lt";
        case 0x18: return "lte";
        case 0x19: return "gt";
        case 0x1A: return "gte";
        case 0x1B: return "alloc";
        case 0x1C: return "free";
        case 0x1D: return "drefr";
        case 0x1E: return "drefw";
        case 0x1F: return "push";
        case 0x20: return "pop";
        case 0x21: return "movbp";
        case 0x22: return "movpb";
    }
    return "Invalid opcode";
}

int is_twobyte(uint8_t opcode) {
    return opcode == 0x02 || opcode == 0x03 || opcode == 0x0f || opcode == 0x11 || opcode == 0x13 || opcode == 0x14 || opcode == 0x15;
}

void format_args(uint8_t op, uint8_t a, uint8_t b, uint8_t c, uint32_t q, uint32_t pos) {
    printf("%04x: \t%s\t", pos, op_name(op));
    char sign = '+';
    int16_t sn = (signed)(b | c << 8);
    if (sn < 0) { 
        sn = -sn;
        sign = '-';
    }
    switch (op) {
        case 0x00:  printf("r%d -> r%d\n", a, b); break;
        case 0x01:  printf("%d -> r%d\n", (b | c << 8), a); break;
        case 0x02:  printf("%d, r%d\n", q, b); break;
        case 0x03:  printf("%d\n", q); break;
        case 0x04:  printf("r%d\n", a); break;
        case 0x05:  printf("r%d, r%d -> r%d\n", a, b, c); break;
        case 0x06:  printf("r%d, %d\n", a, (b | c << 8)); break;
        case 0x07:  printf("r%d, r%d -> r%d\n", a, b, c); break;
        case 0x08:  printf("r%d, %d\n", a, (b | c << 8)); break;
        case 0x09:  printf("r%d, r%d -> r%d\n", a, b, c); break;
        case 0x0a:  printf("r%d, %d\n", a, (b | c << 8)); break;
        case 0x0b:  printf("r%d, r%d -> r%d\n", a, b, c); break;
        case 0x0c:  printf("r%d, %d\n", a, (b | c << 8)); break;
        case 0x0d:  printf("r%d, r%d -> r%d\n", a, b, c); break;
        case 0x0e:  printf("r%d, %d\n", a, (b | c << 8)); break;
        case 0x0f:  printf("%d -> r%d\n", q, a); break;
        case 0x10:  printf("\n"); break;
        case 0x11:  printf("%d\n", q); break;
        case 0x12:  printf("\n"); break;
        case 0x13:  printf("r%d -> %d\n", a, q); break;
        case 0x14:  printf("%d -> r%d\n", q, a); break;
        case 0x15:  printf("%d, r%d\n", q, b); break;
        case 0x16:  printf("r%d, r%d -> r%d\n", a, b, c); break;
        case 0x17:  printf("r%d, r%d -> r%d\n", a, b, c); break;
        case 0x18:  printf("r%d, r%d -> r%d\n", a, b, c); break;
        case 0x19:  printf("r%d, r%d -> r%d\n", a, b, c); break;
        case 0x1A:  printf("r%d, r%d -> r%d\n", a, b, c); break;
        case 0x1B:  printf("r%d\n", a); break;
        case 0x1C:  printf("r%d\n", a); break;
        case 0x1D:  printf("[r%d] -> r%d\n", a, b); break;
        case 0x1E:  printf("r%d -> [r%d]\n", a, b); break;
        case 0x1F:  printf("r%d\n", a); break;
        case 0x20:  printf("r%d\n", a); break;
        case 0x21:  printf("[rbp %c %d] -> r%d\n", sign, sn, a); break;
        case 0x22:  printf("r%d -> [rbp %c %d]\n", a, sign, sn); break;

    }
}

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("provide exactly one file please lol\n");
        return 1;
    }
    char *fname = argv[1];
    
    FILE *fp = fopen(fname, "r");
    if (fp == NULL) {
        printf("file does not exist xx\n");
        return 1;
    }
    fseek(fp, 0, SEEK_END);
    size_t fsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    uint8_t *contents = malloc(fsize);
    fread(contents, 1, fsize, fp);
    fclose(fp);

    struct LinkTable table;
    table_init(&table);
    uint8_t *code = NULL;
    size_t codelen = jobj_process_file(contents, fsize, &code, &table);

    printf("%s defines the following symbols\n", fname);
    for (size_t i = 0; i < table.res_cap; i++) {
        printf("%04x: %s\n", table.locs[i], table.res_syms[i]);
    }

    printf("\nand requires the definition of the following symbols\n");
    for (size_t i = 0; i < table.unres_cap; i++) {
        printf("%s (id %04x)\n", table.unres_syms[i], table.ids[i]);
    }

    printf("\nThe following offsets contain IDs of required symbols\n");
    for (size_t i = 0; i < table.phoff_cap; i++) {
        printf("ID at %04x: %04x\n", table.phoff[i], *(uint32_t*)(code + table.phoff[i]));
    }

    printf("\nBinary code defined in this file:\n");

    for (size_t i = 0; i < codelen; i += 4) {
        printf("%04x: %02x %02x %02x %02x\n", (int)i, code[i], code[i + 1], code[i + 2], code[i + 3]);
    }

    printf("\nDisassembly of binary code:\n");
    for (size_t i = 0; i < codelen;) {
        uint8_t op = code[i];
        uint8_t a = code[i + 1];
        uint8_t b = code[i + 2];
        uint8_t c = code[i + 3];
        uint32_t qwordop;
        uint32_t pos = i;
        i += 4;
        if (is_twobyte(op)) {
            qwordop = *(uint32_t*)(code + i);
            i += 4;
        }
        format_args(op, a, b, c, qwordop, pos);
    }

    free(contents);

    return 0;
}
