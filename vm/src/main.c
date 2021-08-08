#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

struct Reg {
    union {
        char *ptr;
        int i;
    };
};

enum {
    FLAG_CARRY,
    FLAG_ZERO,
    FLAG_NCARRY,
    FLAG_NZERO,
};

struct Machine {
    struct Reg regs[16];
    int flags[4];
    int pc;
    int sp;
    int bp;
};

struct Instruction {
    uint8_t opcode;
    uint8_t arg1;
    union {
        struct {
            uint8_t arg2;
            uint8_t arg3;
        };
        int16_t arg23;
    };
};
/*

0 mov r{arg1} -> r{arg2}
1 movi {arg2 | arg3 << 8} -> r{arg1}
2 jcr cond{arg1}, {arg2 | arg3 << 8}
3 jr {arg2 | arg3 << 8}
4 out r{arg1}

5 add r{arg1}, r{arg2} -> r{arg3}
6 addi r{arg1}, {arg2 | arg3 << 8} -> r{arg1}

7 sub r{arg1}, r{arg2} -> r{arg3}
8 subi r{arg1}, {arg2 | arg3 << 8} -> r{arg1}

9 mul r{arg1}, r{arg2} -> r{arg3}
a muli r{arg1}, {arg2 | arg3 << 8} -> r{arg1}

b div r{arg1}, r{arg2} -> r{arg3}
c divi r{arg1}, {arg2 | arg3 << 8} -> r{arg1}

d mod r{arg1}, r{arg2} -> r{arg3}
e modi r{arg1}, {arg2 | arg3 << 8} -> r{arg1}

f movl {next qword} -> r{arg1}

10 halt


0f 00 00 00
7f ff ff ff
00 00 01 00
01 02 02 00
0c 00 02 00
06 00 02 00
0d 01 02 03
02 01 ff ff
09 02 01 03
02 01 04 00
01 00 01 00
04 00 00 00
03 00 03 00
01 00 00 00
04 00 00 00
10 00 00 00
*/

const char *op_name(uint8_t op) {
    switch (op) {
        case 0x00: return "mov";
        case 0x01: return "movi";
        case 0x02: return "jcr";
        case 0x03: return "jr";
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
        return "Invalid opcode";
    }
}

void run(struct Machine *m, struct Instruction *stream, size_t ninstr) {
    m->pc = 0;
    while ((unsigned)m->pc < ninstr) {
        struct Instruction instr = stream[m->pc];
        //printf("Regs: %d %d %d %d, opcode: %s %d, %d, %d (%d)\n", m->regs[0].i, m->regs[1].i, m->regs[2].i, m->regs[3].i, op_name(instr.opcode), instr.arg1, instr.arg2, instr.arg3, (signed)instr.arg23);
        int flg = 0;
        int reg = 0;
        switch (instr.opcode) {
            case 0x00:
                m->regs[instr.arg2] = m->regs[instr.arg1];
                break;
            case 0x01:
                m->regs[instr.arg1].i = instr.arg23;
                break;
            case 0x02:
                if (m->flags[instr.arg1]) {
                    m->pc += ((signed)instr.arg23) - 1;
                }
                break;
            case 0x03:
                m->pc += (signed)instr.arg23 - 1;
                break;
            case 0x04:
                printf("%d\n", m->regs[instr.arg1].i);
                break;
            case 0x05: m->regs[instr.arg3].i = m->regs[instr.arg1].i + m->regs[instr.arg2].i; flg = 1; reg = m->regs[instr.arg3].i; break;
            case 0x06: m->regs[instr.arg1].i = m->regs[instr.arg1].i + instr.arg23;           flg = 1; reg = m->regs[instr.arg1].i; break;
            case 0x07: m->regs[instr.arg3].i = m->regs[instr.arg1].i - m->regs[instr.arg2].i; flg = 1; reg = m->regs[instr.arg3].i; break;
            case 0x08: m->regs[instr.arg1].i = m->regs[instr.arg1].i - instr.arg23;           flg = 1; reg = m->regs[instr.arg1].i; break;
            case 0x09: m->regs[instr.arg3].i = m->regs[instr.arg1].i * m->regs[instr.arg2].i; flg = 1; reg = m->regs[instr.arg3].i; break;
            case 0x0a: m->regs[instr.arg1].i = m->regs[instr.arg1].i * instr.arg23;           flg = 1; reg = m->regs[instr.arg1].i; break;
            case 0x0b: m->regs[instr.arg3].i = m->regs[instr.arg1].i / m->regs[instr.arg2].i; flg = 1; reg = m->regs[instr.arg3].i; break;
            case 0x0c: m->regs[instr.arg1].i = m->regs[instr.arg1].i / instr.arg23;           flg = 1; reg = m->regs[instr.arg1].i; break;
            case 0x0d: m->regs[instr.arg3].i = m->regs[instr.arg1].i % m->regs[instr.arg2].i; flg = 1; reg = m->regs[instr.arg3].i; break;
            case 0x0e: m->regs[instr.arg1].i = m->regs[instr.arg1].i % instr.arg23;           flg = 1; reg = m->regs[instr.arg1].i; break;

            case 0x0f:
                m->regs[instr.arg1].i = *((uint32_t*)(stream + ++m->pc));
                break;
            case 0x10:
                return;
        }
        if (flg) {
            if (reg == 0) {
                m->flags[FLAG_ZERO] = 1;
            } else {
                m->flags[FLAG_ZERO] = 0;
            }
            m->flags[FLAG_NZERO] = !m->flags[FLAG_ZERO];
        }
        m->pc++;
    }
}

int main() {
    FILE *fp = fopen("test.vm", "r");
    if (fp == NULL) {
        return 1;
    }
    fseek(fp, 0, SEEK_END);
    size_t fsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    uint8_t *contents = malloc(fsize);
    fread(contents, 1, fsize, fp);
    fclose(fp);

    struct Instruction *instrs = (struct Instruction *)contents;
    size_t ninstr = fsize / sizeof(struct Instruction);
    struct Machine m;
    run(&m, instrs, ninstr);
    return 0;
}

