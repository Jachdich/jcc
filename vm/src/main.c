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
    //struct Reg regs[16];
    int32_t regs[16];
    int flags[4];
    int retstk[256];
    int pcsp;
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
        case 0x00: return "mov  ";
        case 0x01: return "movi ";
        case 0x02: return "jz   ";
        case 0x03: return "jp   ";
        case 0x04: return "out  ";
        case 0x05: return "add  ";
        case 0x06: return "addi ";
        case 0x07: return "sub  ";
        case 0x08: return "subi ";
        case 0x09: return "mul  ";
        case 0x0a: return "muli ";
        case 0x0b: return "div  ";
        case 0x0c: return "divi ";
        case 0x0d: return "mod  ";
        case 0x0e: return "modi ";
        case 0x0f: return "movl ";
        case 0x10: return "halt ";
        case 0x11: return "call ";
        case 0x12: return "ret  ";
        case 0x13: return "movra";
        case 0x14: return "movar";
        case 0x15: return "jnz  ";
        case 0x16: return "cmp  ";
        case 0x17: return "lt   ";
        case 0x18: return "lte  ";
        case 0x19: return "gt   ";
        case 0x1A: return "gte  ";
        case 0x1B: return "alloc";
        case 0x1C: return "free ";
        case 0x1D: return "drefr";
        case 0x1E: return "drefw";
    }
    return "Invalid opcode";
}

void run(struct Machine *m, struct Instruction *stream, size_t ninstr) {
    m->pc = 0;
    m->pcsp = 0;
    while ((unsigned)m->pc < ninstr) {
        struct Instruction instr = stream[m->pc++];
        printf("Pc: %02x, Regs: %08x %08x %08x %08x, opcode: %s %02x, %02x, %02x (%04x) (next qword %08x instr %08x)\n",
                m->pc, m->regs[0], m->regs[1], m->regs[2], m->regs[3],
                op_name(instr.opcode), instr.arg1, instr.arg2, instr.arg3,
                (signed)instr.arg23, *((int32_t*)(stream + m->pc)), *((int32_t*)(stream + m->pc)) / 4);

        switch (instr.opcode) {
            case 0x00:
                m->regs[instr.arg2] = m->regs[instr.arg1];
                break;
            case 0x01:
                m->regs[instr.arg1] = instr.arg23;
                break;
            case 0x02:
                if (m->regs[instr.arg1] == 0) {
                    m->pc = *((int32_t*)(stream + m->pc)) / 4 ;
                } else {
                    m->pc++;
                }
                break;
            case 0x15:
                if (m->regs[instr.arg1] != 0) {
                    m->pc = *((int32_t*)(stream + m->pc)) / 4 ;
                } else {
                    m->pc++;
                }
                break;
            case 0x03:
                m->pc = *((int32_t*)(stream + m->pc)) / 4;
                break;
            case 0x04:
                printf("%d\n", m->regs[instr.arg1]);
                break;
            case 0x05: m->regs[instr.arg3] = m->regs[instr.arg1] + m->regs[instr.arg2]; break;
            case 0x06: m->regs[instr.arg1] = m->regs[instr.arg1] + instr.arg23;         break;
            case 0x07: m->regs[instr.arg3] = m->regs[instr.arg1] - m->regs[instr.arg2]; break;
            case 0x08: m->regs[instr.arg1] = m->regs[instr.arg1] - instr.arg23;         break;
            case 0x09: m->regs[instr.arg3] = m->regs[instr.arg1] * m->regs[instr.arg2]; break;
            case 0x0a: m->regs[instr.arg1] = m->regs[instr.arg1] * instr.arg23;         break;
            case 0x0b: m->regs[instr.arg3] = m->regs[instr.arg1] / m->regs[instr.arg2]; break;
            case 0x0c: m->regs[instr.arg1] = m->regs[instr.arg1] / instr.arg23;         break;
            case 0x0d: m->regs[instr.arg3] = m->regs[instr.arg1] % m->regs[instr.arg2]; break;
            case 0x0e: m->regs[instr.arg1] = m->regs[instr.arg1] % instr.arg23;         break;

            case 0x0f:
                m->regs[instr.arg1] = *((uint32_t*)(stream + m->pc++));
                break;
            case 0x10:
                return;
            case 0x11: {
                uint32_t addr = *((uint32_t*)(stream + m->pc++));
                m->retstk[m->pcsp++] = m->pc;
                m->pc = addr / 4;
                break;
            }

            case 0x12:
                m->pc = m->retstk[--m->pcsp];
                break;

            case 0x13: {
                uint32_t addr = *((uint32_t*)(stream + m->pc++));
                *(uint32_t*)((uint8_t*)stream + addr) = m->regs[instr.arg1];
                break;
            }

            case 0x14: {
                uint32_t addr = *((uint32_t*)(stream + m->pc++));
                //assume relative address since it is a constant
                m->regs[instr.arg1] = *(uint32_t*)((uint8_t*)stream + addr);
                break;
            }
            case 0x16: m->regs[instr.arg3] = m->regs[instr.arg1] == m->regs[instr.arg2]; break;
            case 0x17: m->regs[instr.arg3] = m->regs[instr.arg1] <  m->regs[instr.arg2]; break;
            case 0x18: m->regs[instr.arg3] = m->regs[instr.arg1] <= m->regs[instr.arg2]; break;
            case 0x19: m->regs[instr.arg3] = m->regs[instr.arg1] >  m->regs[instr.arg2]; break;
            case 0x1A: m->regs[instr.arg3] = m->regs[instr.arg1] >= m->regs[instr.arg2]; break;
            case 0x1B: m->regs[instr.arg1] = (int32_t)(size_t)malloc(*((uint32_t*)(stream + m->pc++))) | 0x80000000; break;
            case 0x1C: free((void*)(size_t)(m->regs[instr.arg1] & 0x7FFFFFFF)); break;
            case 0x1D: {
                uint32_t addr = m->regs[instr.arg1];
                if (addr & 0x80000000) {
                    //absolute address
                    m->regs[instr.arg2] = *(uint32_t*)((size_t)addr & 0x7FFFFFFF);
                } else {
                    //relative address
                    m->regs[instr.arg2] = *(uint32_t*)((uint8_t*)stream + addr);
                }
                break;
            }

            case 0x1E: {
                uint32_t addr = m->regs[instr.arg2];
                if (addr & 0x80000000) {
                    //absolute address
                    *(uint32_t*)((size_t)addr & 0x7FFFFFFF) = m->regs[instr.arg1];
                } else {
                    //relative address
                    *(uint32_t*)((uint8_t*)stream + addr) = m->regs[instr.arg1];
                }
                break;
            }
            default:
                printf("Error: unrecognised opcode %02x\n", instr.opcode);
                exit(0);
        }
    }
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s filename\n", argv[0]);
        return 1;
    }
    FILE *fp = fopen(argv[1], "r");
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
    for (uint8_t i = 0; i < 16; i++) {
        m.regs[i] = 0;
    }
    run(&m, instrs, ninstr);
    return 0;
}