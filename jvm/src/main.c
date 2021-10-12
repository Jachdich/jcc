#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#define SP_POS 16
#define BP_POS 17
#define SETBITS   0x80000000
#define UNSETBITS 0x7FFFFFFF

struct Machine {
    ssize_t regs[18];
    int retstk[256];
    int pcsp;
    int pc;
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

const char *op_name(uint8_t op) {
    switch (op) {
        case 0x01: return "movi  ";
        case 0x00: return "mov   ";
        case 0x02: return "jz    ";
        case 0x03: return "jp    ";
        case 0x04: return "out   ";
        case 0x05: return "add   ";
        case 0x06: return "addi  ";
        case 0x07: return "sub   ";
        case 0x08: return "subi  ";
        case 0x09: return "mul   ";
        case 0x0a: return "muli  ";
        case 0x0b: return "div   ";
        case 0x0c: return "divi  ";
        case 0x0d: return "mod   ";
        case 0x0e: return "modi  ";
        case 0x0f: return "movl  ";
        case 0x10: return "halt  ";
        case 0x11: return "call  ";
        case 0x12: return "ret   ";
        case 0x13: return "movrab";
        case 0x14: return "movarb";
        case 0x15: return "jnz   ";
        case 0x16: return "cmp   ";
        case 0x17: return "lt    ";
        case 0x18: return "lte   ";
        case 0x19: return "gt    ";
        case 0x1A: return "gte   ";
        case 0x1B: return "alloc ";
        case 0x1C: return "free  ";
        case 0x1D: return "drefrb";
        case 0x1E: return "drefwb";
        case 0x1F: return "pushb ";
        case 0x20: return "popb  ";
        case 0x21: return "movbpb";
        case 0x22: return "mobpbb";
        case 0x23: return "movbpd";
        case 0x24: return "mobpbd";
        case 0x25: return "movbpq";
        case 0x26: return "mobpbq";
        case 0x27: return "drefrd";
        case 0x28: return "drefwd";
        case 0x29: return "drefrq";
        case 0x2A: return "drefwq";
        case 0x2B: return "movrad";
        case 0x2C: return "movard";
        case 0x2D: return "movraq";
        case 0x2E: return "movarq";
        case 0x2F: return "pushd ";
        case 0x30: return "popd  ";
        case 0x31: return "pushq ";
        case 0x32: return "popq  ";
    }
    return "Invalid opcode";
}

const char reset[] = {0x1B, '[', '0', 'm', 0};
const char underline[] = {0x1B, '[', '4','m', 0};
const char invert[] = {0x1B, '[', '7','m', 0};

void run(struct Machine *m, struct Instruction *stream, size_t ninstr, uint8_t *stack) {
    m->pc = 0;
    m->pcsp = 0;
    while ((unsigned)m->pc < ninstr) {
        struct Instruction instr = stream[m->pc++];
        /*printf("Pc: %02x, Regs: %08x %08x %08x %08x, opcode: %s %02x, %02x, %02x (%04x) (next qword %08x instr %08x) ",
                m->pc, (int32_t)m->regs[0], (int32_t)m->regs[1], (int32_t)m->regs[2], (int32_t)m->regs[3],
                op_name(instr.opcode), instr.arg1, instr.arg2, instr.arg3,
                (signed)instr.arg23, *((int32_t*)(stream + m->pc)), *((int32_t*)(stream + m->pc)) / 4);

        for (int i = 0; i < 32; i++) {
            if (((size_t)(stack + i) | SETBITS) == (unsigned)m->regs[SP_POS]) {
                printf(underline);
            }
            if (((size_t)(stack + i) | SETBITS) == (unsigned)m->regs[BP_POS]) {
                printf(invert);
            }
            printf("%02x%s ", stack[i], reset);
            if ((i + 1) % 4 == 0) {
                printf(" ");
            }
        }
        printf("BP, SP: %lu %lu\n", m->regs[SP_POS], m->regs[BP_POS]);*/

        switch (instr.opcode) {
            case 0x00: //mov
                m->regs[instr.arg2] = m->regs[instr.arg1];
                break;
            case 0x01: //movi
                m->regs[instr.arg1] = instr.arg23;
                break;
            case 0x02: //jz
                if (m->regs[instr.arg1] == 0) {
                    m->pc = *((int32_t*)(stream + m->pc)) / 4;
                } else {
                    m->pc++;
                }
                break;
            case 0x15: //jnz
                if (m->regs[instr.arg1] != 0) {
                    m->pc = *((int32_t*)(stream + m->pc)) / 4 ;
                } else {
                    m->pc++;
                }
                break;
            case 0x03:  //jp
                m->pc = *((int32_t*)(stream + m->pc)) / 4;
                break;
            case 0x04: //out
                printf("%d\n", (int32_t)m->regs[instr.arg1]);
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

            case 0x0f: //movl
                m->regs[instr.arg1] = *((uint32_t*)(stream + m->pc++));
                break;
            case 0x10: //halt
                return;
            
            case 0x11: { //call
                uint32_t addr = *((uint32_t*)(stream + m->pc++));
                m->retstk[m->pcsp++] = m->pc;
                m->pc = addr / 4;
                break;
            }

            case 0x12: //ret
                m->pc = m->retstk[--m->pcsp];
                break;

            case 0x13: { //movrab
                uint32_t addr = *((uint32_t*)(stream + m->pc++));
                *((uint8_t*)stream + addr) = m->regs[instr.arg1];
                break;
            }

            case 0x14: { //movarb
                size_t addr = *((uint32_t*)(stream + m->pc++));
                //assume relative address since it is a constant
                m->regs[instr.arg1] = *(uint8_t*)((uint8_t*)stream + addr);
                break;
            }
            case 0x16: m->regs[instr.arg3] = m->regs[instr.arg1] == m->regs[instr.arg2]; break;
            case 0x17: m->regs[instr.arg3] = m->regs[instr.arg1] <  m->regs[instr.arg2]; break;
            case 0x18: m->regs[instr.arg3] = m->regs[instr.arg1] <= m->regs[instr.arg2]; break;
            case 0x19: m->regs[instr.arg3] = m->regs[instr.arg1] >  m->regs[instr.arg2]; break;
            case 0x1A: m->regs[instr.arg3] = m->regs[instr.arg1] >= m->regs[instr.arg2]; break;
            case 0x1B: m->regs[instr.arg1] = (size_t)malloc(m->regs[instr.arg2]) | SETBITS; break;
            case 0x1C: free((void*)(size_t)(m->regs[instr.arg1] & UNSETBITS)); break;
            case 0x1D: { //drefrb
                size_t addr = m->regs[instr.arg1];
                if (addr & SETBITS) {
                    //absolute address
                    m->regs[instr.arg2] = *(uint8_t*)(addr & UNSETBITS);
                } else {
                    //relative address
                    m->regs[instr.arg2] = *(uint8_t*)((uint8_t*)stream + addr);
                }
                break;
            }

            case 0x1E: { //drefwb
                size_t addr = m->regs[instr.arg2];
                if (addr & SETBITS) {
                    //absolute address
                    *(uint8_t*)((size_t)addr & UNSETBITS) = m->regs[instr.arg1];
                } else {
                    //relative address
                    *(uint8_t*)((uint8_t*)stream + addr) = m->regs[instr.arg1];
                }
                break;
            }
            case 0x1F: //pushb
                if (m->regs[SP_POS] & SETBITS) {
                    *(uint8_t*)((size_t)m->regs[SP_POS] & UNSETBITS) = m->regs[instr.arg1];
                    m->regs[SP_POS] += 4;
                } else {
                    *(uint8_t*)((uint8_t*)stream + m->regs[SP_POS]) = m->regs[instr.arg1];
                    m->regs[SP_POS] += 4;
                }
                break;
            case 0x20: //popb
                if (m->regs[SP_POS] & SETBITS) {
                    m->regs[SP_POS] -= 4;
                    m->regs[instr.arg1] = *(uint8_t*)((size_t)m->regs[SP_POS] & UNSETBITS);
                } else {
                    m->regs[SP_POS] -= 4;
                    m->regs[instr.arg1] = *(uint8_t*)((uint8_t*)stream + m->regs[SP_POS]);
                }
                break;
            case 0x21: { //movbpb
                size_t addr = (m->regs[BP_POS] + (signed)instr.arg23);
                if (addr & SETBITS) {
                    m->regs[instr.arg1] = *(uint8_t*)(size_t)(addr & UNSETBITS);
                } else {
                    m->regs[instr.arg1] = *(uint8_t*)((uint8_t*)stream + addr);
                }
                break;
            }
            case 0x22: { //movpbb
                size_t addr = (m->regs[BP_POS] + (signed)instr.arg23);
                if (addr & SETBITS) {
                    *(uint8_t*)(size_t)(addr & UNSETBITS) = m->regs[instr.arg1];
                } else {
                    *(uint8_t*)((uint8_t*)stream + addr) = m->regs[instr.arg1];
                }
                break;
            }
            case 0x23: { //movbpd
                size_t addr = (m->regs[BP_POS] + (signed)instr.arg23);
                if (addr & SETBITS) {
                    m->regs[instr.arg1] = *(uint16_t*)(size_t)(addr & UNSETBITS);
                } else {
                    m->regs[instr.arg1] = *(uint16_t*)((uint8_t*)stream + addr);
                }
                break;
            }
            case 0x24: { //movpbd
                size_t addr = (m->regs[BP_POS] + (signed)instr.arg23);
                if (addr & SETBITS) {
                    *(uint16_t*)(size_t)(addr & UNSETBITS) = m->regs[instr.arg1];
                } else {
                    *(uint16_t*)((uint8_t*)stream + addr) = m->regs[instr.arg1];
                }
                break;
            }
            case 0x25: { //movbpq
                size_t addr = (m->regs[BP_POS] + (signed)instr.arg23);
                if (addr & SETBITS) {
                    m->regs[instr.arg1] = *(uint32_t*)(size_t)(addr & UNSETBITS);
                } else {
                    m->regs[instr.arg1] = *(uint32_t*)((uint8_t*)stream + addr);
                }
                break;
            }
            case 0x26: { //movpbq
                size_t addr = (m->regs[BP_POS] + (signed)instr.arg23);
                if (addr & SETBITS) {
                    *(uint32_t*)(size_t)(addr & UNSETBITS) = m->regs[instr.arg1];
                } else {
                    *(uint32_t*)((uint8_t*)stream + addr) = m->regs[instr.arg1];
                }
                break;
            }
            case 0x27: { //drefrd
                size_t addr = m->regs[instr.arg1];
                if (addr & SETBITS) {
                    //absolute address
                    m->regs[instr.arg2] = *(uint16_t*)((size_t)addr & UNSETBITS);
                } else {
                    //relative address
                    m->regs[instr.arg2] = *(uint16_t*)((uint8_t*)stream + addr);
                }
                break;
            }

            case 0x28: { //drefwd
                size_t addr = m->regs[instr.arg2];
                if (addr & SETBITS) {
                    //absolute address
                    *(uint16_t*)((size_t)addr & UNSETBITS) = m->regs[instr.arg1];
                } else {
                    //relative address
                    *(uint16_t*)((uint8_t*)stream + addr) = m->regs[instr.arg1];
                }
                break;
            }
            case 0x29: { //drefrq
                size_t addr = m->regs[instr.arg1];
                if (addr & SETBITS) {
                    //absolute address
                    m->regs[instr.arg2] = *(uint32_t*)((size_t)addr & UNSETBITS);
                } else {
                    //relative address
                    m->regs[instr.arg2] = *(uint32_t*)((uint8_t*)stream + addr);
                }
                break;
            }

            case 0x2A: { //drefwq
                size_t addr = m->regs[instr.arg2];
                if (addr & SETBITS) {
                    //absolute address
                    *(uint32_t*)((size_t)addr & UNSETBITS) = m->regs[instr.arg1];
                } else {
                    //relative address
                    *(uint32_t*)((uint8_t*)stream + addr) = m->regs[instr.arg1];
                }
                break;
            }
            case 0x2B: { //movrab
                size_t addr = *((uint32_t*)(stream + m->pc++));
                *(uint16_t*)((uint8_t*)stream + addr) = m->regs[instr.arg1];
                break;
            }

            case 0x2C: { //movarb
                size_t addr = *((uint32_t*)(stream + m->pc++));
                //assume relative address since it is a constant
                m->regs[instr.arg1] = *(uint16_t*)((uint8_t*)stream + addr);
                break;
            }
            case 0x2D: { //movrab
                size_t addr = *((uint32_t*)(stream + m->pc++));
                *(uint32_t*)((uint8_t*)stream + addr) = m->regs[instr.arg1];
                break;
            }

            case 0x2E: { //movarb
                size_t addr = *((uint32_t*)(stream + m->pc++));
                //assume relative address since it is a constant
                m->regs[instr.arg1] = *(uint32_t*)((uint8_t*)stream + addr);
                break;
            }
            case 0x2F: //pushd
                if (m->regs[SP_POS] & SETBITS) {
                    *(uint16_t*)((size_t)m->regs[SP_POS] & UNSETBITS) = m->regs[instr.arg1];
                    m->regs[SP_POS] += 4;
                } else {
                    *(uint16_t*)((uint8_t*)stream + m->regs[SP_POS]) = m->regs[instr.arg1];
                    m->regs[SP_POS] += 4;
                }
                break;
            case 0x30: //popd
                if (m->regs[SP_POS] & SETBITS) {
                    m->regs[SP_POS] -= 4;
                    m->regs[instr.arg1] = *(uint16_t*)((size_t)m->regs[SP_POS] & UNSETBITS);
                } else {
                    m->regs[SP_POS] -= 4;
                    m->regs[instr.arg1] = *(uint16_t*)((uint8_t*)stream + m->regs[SP_POS]);
                }
                break;
            case 0x31: //pushq
                if (m->regs[SP_POS] & SETBITS) {
                    *(uint32_t*)((size_t)m->regs[SP_POS] & UNSETBITS) = m->regs[instr.arg1];
                    m->regs[SP_POS] += 4;
                } else {
                    *(uint32_t*)((uint8_t*)stream + m->regs[SP_POS]) = m->regs[instr.arg1];
                    m->regs[SP_POS] += 4;
                }
                break;
            case 0x32: //popq
                if (m->regs[SP_POS] & SETBITS) {
                    m->regs[SP_POS] -= 4;
                    m->regs[instr.arg1] = *(uint32_t*)((size_t)m->regs[SP_POS] & UNSETBITS);
                } else {
                    m->regs[SP_POS] -= 4;
                    m->regs[instr.arg1] = *(uint32_t*)((uint8_t*)stream + m->regs[SP_POS]);
                }
                break;
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

    uint8_t *stack = malloc(4096);
    
    struct Machine m;
    for (uint8_t i = 0; i < 16; i++) {
        m.regs[i] = 0;
    }
    m.regs[SP_POS] = ((size_t)stack) | SETBITS;
    m.regs[BP_POS] = m.regs[SP_POS];
    run(&m, instrs, ninstr, stack);
    free(contents);
    free(stack);
    return 0;
}
