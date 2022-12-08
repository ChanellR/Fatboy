#include <stdio.h>

#include "header.h"
#include "memory.h"
#include "control.h"
#include "display.h"
#include "gpu.h"


#define CLOCKSPEED 4194304
// #define TIMA 0xFF05
// #define TMA 0xFF06
// #define TAC 0xFF07

struct timer timer;
struct interrupt interrupt;
struct registers registers;
unsigned char stopped;
int frame; //number of Updates performed. 
int currentcycles;

int m_timercounter = CLOCKSPEED / 1024; // set to mode 0
int m_dividercounter = 256;

const struct Instruction instructions[256] = {
    //-3: word LD, -2: byte LD, -1: register specific, 0: none, 1:byte, 2: word

    {0, nop, 1},        // 0x00
    {-3, load16bit, 3}, // 0x01
    {-1, loadRegS, 2},  // 0x02
    {-1, INCRegS, 2},   // 0x03
    {-1, INCRegB, 1},   // 0x04
    {-1, DECRegB, 1},   // 0x05
    {-2, load8bit, 2},  // 0x06
    {-1, RR, 1},        // 0x07
    {-3, load16bit, 5}, // 0x08
    {-1, ADDHL, 2},     // 0x09
    {-1, loadRegS, 2},  // 0x0a
    {-1, DECRegS, 2},   // 0x0b
    {-1, INCRegB, 1},   // 0x0c
    {-1, DECRegB, 1},   // 0x0d
    {-2, load8bit, 2},  // 0x0e
    {-1, RR, 1},        // 0x0f

    {0, STOP, 2},       // 0x00
    {-3, load16bit, 3}, // 0x01
    {-1, loadRegS, 2},  // 0x02
    {-1, INCRegS, 2},   // 0x03
    {-1, INCRegB, 1},   // 0x04
    {-1, DECRegB, 1},   // 0x05
    {-2, load8bit, 2},  // 0x06
    {-1, RR, 1},        // 0x07
    {-2, JUMP8, 3},     // 0x08
    {-1, ADDHL, 2},     // 0x09
    {-1, loadRegS, 2},  // 0x0a
    {-1, DECRegS, 2},   // 0x0b
    {-1, INCRegB, 1},   // 0x0c
    {-1, DECRegB, 1},   // 0x0d
    {-2, load8bit, 2},  // 0x0e
    {-1, RR, 1},        // 0x0f

    {-2, JUMP8, 3},     // 0x00
    {-3, load16bit, 3}, // 0x01
    {-1, loadRegS, 2},  // 0x02
    {-1, INCRegS, 2},   // 0x03
    {-1, INCRegB, 1},   // 0x04
    {-1, DECRegB, 1},   // 0x05
    {-2, load8bit, 2},  // 0x06
    {0, DAA, 1},        // 0x07
    {-2, JUMP8, 3},     // 0x08
    {-1, ADDHL, 2},     // 0x09
    {-1, loadRegS, 2},  // 0x0a
    {-1, DECRegS, 2},   // 0x0b
    {-1, INCRegB, 1},   // 0x0c
    {-1, DECRegB, 1},   // 0x0d
    {-2, load8bit, 2},  // 0x0e
    {0, CPL, 1},        // 0x0f

    {-2, JUMP8, 3},     // 0x00
    {-3, load16bit, 3}, // 0x01
    {-1, loadRegS, 2},  // 0x02
    {-1, INCRegS, 2},   // 0x03
    {-1, INCRegB, 1},   // 0x04
    {-1, DECRegB, 1},   // 0x05
    {-2, load8bit, 2},  // 0x06
    {0, SCF, 1},        // 0x07
    {-2, JUMP8, 3},     // 0x08
    {-1, ADDHL, 2},     // 0x09
    {-1, loadRegS, 2},  // 0x0a
    {-1, DECRegS, 2},   // 0x0b
    {-1, INCRegB, 1},   // 0x0c
    {-1, DECRegB, 1},   // 0x0d
    {-2, load8bit, 2},  // 0x0e
    {0, CCF, 1},        // 0x0f

    {-1, loadRegB, 1}, // 0x00
    {-1, loadRegB, 1}, // 0x01
    {-1, loadRegB, 1}, // 0x02
    {-1, loadRegB, 1}, // 0x03
    {-1, loadRegB, 1}, // 0x04
    {-1, loadRegB, 1}, // 0x05
    {-1, loadRegB, 1}, // 0x06
    {-1, loadRegB, 1}, // 0x07
    {-1, loadRegB, 1}, // 0x08
    {-1, loadRegB, 1}, // 0x09
    {-1, loadRegB, 1}, // 0x0A
    {-1, loadRegB, 1}, // 0x0B
    {-1, loadRegB, 1}, // 0x0C
    {-1, loadRegB, 1}, // 0x0D
    {-1, loadRegB, 1}, // 0x0E
    {-1, loadRegB, 1}, // 0x0F

    {-1, loadRegB, 1}, // 0x00
    {-1, loadRegB, 1}, // 0x01
    {-1, loadRegB, 1}, // 0x02
    {-1, loadRegB, 1}, // 0x03
    {-1, loadRegB, 1}, // 0x04
    {-1, loadRegB, 1}, // 0x05
    {-1, loadRegB, 1}, // 0x06
    {-1, loadRegB, 1}, // 0x07
    {-1, loadRegB, 1}, // 0x08
    {-1, loadRegB, 1}, // 0x09
    {-1, loadRegB, 1}, // 0x0A
    {-1, loadRegB, 1}, // 0x0B
    {-1, loadRegB, 1}, // 0x0C
    {-1, loadRegB, 1}, // 0x0D
    {-1, loadRegB, 1}, // 0x0E
    {-1, loadRegB, 1}, // 0x0F

    {-1, loadRegB, 1}, // 0x00
    {-1, loadRegB, 1}, // 0x01
    {-1, loadRegB, 1}, // 0x02
    {-1, loadRegB, 1}, // 0x03
    {-1, loadRegB, 1}, // 0x04
    {-1, loadRegB, 1}, // 0x05
    {-1, loadRegB, 1}, // 0x06
    {-1, loadRegB, 1}, // 0x07
    {-1, loadRegB, 1}, // 0x08
    {-1, loadRegB, 1}, // 0x09
    {-1, loadRegB, 1}, // 0x0A
    {-1, loadRegB, 1}, // 0x0B
    {-1, loadRegB, 1}, // 0x0C
    {-1, loadRegB, 1}, // 0x0D
    {-1, loadRegB, 1}, // 0x0E
    {-1, loadRegB, 1}, // 0x0F

    {-1, loadRegB, 1}, // 0x00
    {-1, loadRegB, 1}, // 0x01
    {-1, loadRegB, 1}, // 0x02
    {-1, loadRegB, 1}, // 0x03
    {-1, loadRegB, 1}, // 0x04
    {-1, loadRegB, 1}, // 0x05
    {0, HALT, 1},      // 0x06
    {-1, loadRegB, 1}, // 0x07
    {-1, loadRegB, 1}, // 0x08
    {-1, loadRegB, 1}, // 0x09
    {-1, loadRegB, 1}, // 0x0A
    {-1, loadRegB, 1}, // 0x0B
    {-1, loadRegB, 1}, // 0x0C
    {-1, loadRegB, 1}, // 0x0D
    {-1, loadRegB, 1}, // 0x0E
    {-1, loadRegB, 1}, // 0x0F

    {-1, ADD, 1}, // 0x00
    {-1, ADD, 1}, // 0x01
    {-1, ADD, 1}, // 0x02
    {-1, ADD, 1}, // 0x03
    {-1, ADD, 1}, // 0x04
    {-1, ADD, 1}, // 0x05
    {-1, ADD, 1}, // 0x06
    {-1, ADD, 1}, // 0x07
    {-1, ADD, 1}, // 0x08
    {-1, ADD, 1}, // 0x09
    {-1, ADD, 1}, // 0x0A
    {-1, ADD, 1}, // 0x0B
    {-1, ADD, 1}, // 0x0C
    {-1, ADD, 1}, // 0x0D
    {-1, ADD, 1}, // 0x0E
    {-1, ADD, 1}, // 0x0F

    {-1, SUB, 1}, // 0x00
    {-1, SUB, 1}, // 0x01
    {-1, SUB, 1}, // 0x02
    {-1, SUB, 1}, // 0x03
    {-1, SUB, 1}, // 0x04
    {-1, SUB, 1}, // 0x05
    {-1, SUB, 1}, // 0x06
    {-1, SUB, 1}, // 0x07
    {-1, SUB, 1}, // 0x08
    {-1, SUB, 1}, // 0x09
    {-1, SUB, 1}, // 0x0A
    {-1, SUB, 1}, // 0x0B
    {-1, SUB, 1}, // 0x0C
    {-1, SUB, 1}, // 0x0D
    {-1, SUB, 1}, // 0x0E
    {-1, SUB, 1}, // 0x0F

    {-1, AND, 1}, // 0x0
    {-1, AND, 1},
    {-1, AND, 1},
    {-1, AND, 1},
    {-1, AND, 1},
    {-1, AND, 1},
    {-1, AND, 1},
    {-1, AND, 1},

    {-1, XOR, 1}, // 0x08
    {-1, XOR, 1},
    {-1, XOR, 1},
    {-1, XOR, 1},
    {-1, XOR, 1},
    {-1, XOR, 1},
    {-1, XOR, 1},
    {-1, XOR, 1},

    {-1, OR, 1}, // 0x00
    {-1, OR, 1},
    {-1, OR, 1},
    {-1, OR, 1},
    {-1, OR, 1},
    {-1, OR, 1},
    {-1, OR, 1},
    {-1, OR, 1},

    {-1, CP, 1}, // 0x08
    {-1, CP, 1},
    {-1, CP, 1},
    {-1, CP, 1},
    {-1, CP, 1},
    {-1, CP, 1},
    {-1, CP, 1},
    {-1, CP, 1},
    /// Cx
    {-1, RET, 5}, // 0x00
    {-1, POP, 3},
    {-3, JUMP16, 4},
    {-3, JUMP16, 4},
    {-3, CALL, 6},
    {-1, PUSH, 4},
    {-2, ADD8, 2},
    {-1, RST, 4},

    {-1, RET, 5}, // 0x08
    {-1, RET, 4},
    {-3, JUMP16, 4},
    {-2, CB, 2},
    {-3, CALL, 6},
    {-3, CALL, 6},
    {-2, ADD8, 1},
    {-1, RST, 4},
    /// Dx
    {-1, RET, 5}, // 0x00
    {-1, POP, 3},
    {-3, JUMP16, 4},
    {0, nop, 1},
    {-3, CALL, 6},
    {-1, PUSH, 4},
    {-2, SUB8, 2},
    {-1, RST, 4},

    {-1, RET, 5}, // 0x08
    {-1, RETI, 4},
    {-3, JUMP16, 4},
    {0, nop, 1},
    {-3, CALL, 6},
    {0, nop, 1},
    {-2, SUB8, 2},
    {-1, RST, 4},
    // Ex
    {-2, load8bit, 3}, // 0x00
    {-1, POP, 3},
    {-1, load8bit, 2},
    {0, nop, 1},
    {0, nop, 1},
    {-1, PUSH, 4},
    {-2, AND8, 2},
    {-1, RST, 4},

    {-4, ADDSP, 4}, // 0x08
    {0, JUMPHL, 1},
    {-3, load16bit, 4},
    {0, nop, 1},
    {0, nop, 1},
    {0, nop, 1},
    {-2, XOR8, 2},
    {-1, RST, 4},
    // Fx
    {-2, load8bit, 3}, // 0x00
    {-1, POP, 3},
    {-3, load8bit, 2},
    {0, DI, 1},
    {0, nop, 1},
    {-1, PUSH, 4},
    {-2, OR8, 2},
    {-1, RST, 4},

    {-4, ADDSP, 3}, // 0x08
    {-1, loadRegS, 2},
    {-3, load16bit, 4},
    {0, EI, 1},
    {0, nop, 1},
    {0, nop, 1},
    {-2, CP8, 2},
    {-1, RST, 4}

};

int CpuStep(void)
{

    unsigned char instruction = ReadByte(registers.PC++);

    switch (instructions[instruction].operand_length)
    {

    case -4:
        ((void (*)(unsigned char, signed char))instructions[instruction].function)(instruction, ReadByte(registers.PC++));
    case -3:
        ((void (*)(unsigned char, unsigned short))instructions[instruction].function)(instruction, ReadShort(registers.PC));
        registers.PC += 2;
        break;
    case -2:
        ((void (*)(unsigned char, unsigned char))instructions[instruction].function)(instruction, ReadByte(registers.PC++));
        break;
    case -1:
        ((void (*)(unsigned char))instructions[instruction].function)(instruction);
        break;

    case 0:
        ((void (*)())instructions[instruction].function)();
        break;
    case 1:
        ((void (*)(unsigned char))instructions[instruction].function)(ReadByte(registers.PC++));
        break;
    }

    return instructions[instruction].cycles;
}

// BC: 000, DE: 010, HL: 100, SP: 110 r1
//  B: 000, D: 010, H:100, (HL): 110 r1
//  C: 001, E: 011, L: 101, A:111
unsigned short *topRowRegs[7] = {&registers.BC, NULL, &registers.DE, NULL, &registers.HL, NULL, &registers.SP};
unsigned char *topRowRegsHigh[6] = {&registers.B, NULL, &registers.D, NULL, &registers.H, NULL}; //(HL) should be at end
unsigned char *topRowRegsHigh2[8] = {NULL, &registers.C, NULL, &registers.E, NULL, &registers.L, NULL, &registers.A};

void nop(void) {}

void STOP(void)
{
    stopped = 1;
    printf("Stopped");
    // Reset, start at 0000h
    // input, start after instruction
}

void HALT(void)
{
    stopped = 2;
    if (!interrupt.flag){ //if no interrupts, move backwards
        registers.PC--;
    }
    // Halted
}

void DAA(void)
{
    // note: assumes a is a uint8_t and wraps from 0xff to 0
    signed char additive;
    unsigned char done = 0;

    if (!FLAG_ISSET(FLAG_N))
    { // after an addition, adjust if (half-)carry occurred or if result is out of bounds
        if ( ( ( (registers.A & 0xF0) > 0x80 ) && ((registers.A & 0x0F) > 0x9)) )
        {
            additive += 0x66;
            done = 1;
            FLAG_SET(FLAG_CARRY);
        }
            
        
        if ( (FLAG_ISSET(FLAG_CARRY) || ((registers.A & 0XF0) > 0x90)) && !done)
        {   //hardcode 9-F and A-F as 66
            additive += 0x60;
            FLAG_SET(FLAG_CARRY);
        }
        if ( (FLAG_ISSET(FLAG_HALF) || ((registers.A & 0x0F) > 0x09)) && !done)
        {
            additive += 0x6;
        }
    }
    else
    { // after a subtraction, only adjust if (half-)carry occurred
        if (FLAG_ISSET(FLAG_CARRY))
        {
            additive -= 0x60;
        }
        if (FLAG_ISSET(FLAG_HALF))
        {
            additive -= 0x6;
        }
        if(additive <= -0x60) {FLAG_SET(FLAG_CARRY);}
    }
    // these flags are always updated
    registers.A += additive;

    if(registers.A == 0) {FLAG_SET(FLAG_ZERO);} else {FLAG_CLEAR(FLAG_ZERO);} // the usual z flag
    FLAG_CLEAR(FLAG_HALF);                         // h flag is always cleared

    CreateBox("after DAA");
}

void CPL(void)
{

    registers.A &= 0xFF;
}

void SCF(void)
{

    FLAG_SET(FLAG_CARRY);
}

void CCF(void)
{

    if (FLAG_ISSET(FLAG_CARRY))
    {
        FLAG_CLEAR(FLAG_CARRY);
    }
    else
    {
        FLAG_SET(FLAG_CARRY);
    }
}

void DI(void)
{
    interrupt.master = 0;
}

void EI(void)
{
    interrupt.master = 1;
}

void AND(unsigned char Opcode)
{

    unsigned char *BitRegs[8] = {&registers.B, &registers.C, &registers.D, &registers.E, &registers.H, &registers.L, NULL, &registers.A};
    struct opcode and;
    and.inst = Opcode;
    unsigned char val;

    if (and.r2 == 6)
    {
        val = ReadByte(registers.HL);
    }
    else
    {
        val = *BitRegs[and.r2];
    }

    registers.A &= val;

    if (registers.A == 0)
    {
        FLAG_SET(FLAG_ZERO);
    }
    else
    {
        FLAG_CLEAR(FLAG_ZERO);
    } // Z
    FLAG_CLEAR(FLAG_N);
    FLAG_SET(FLAG_HALF);
    FLAG_CLEAR(FLAG_CARRY);
    CreateBox("After AND");
}

void XOR(unsigned char Opcode)
{

    unsigned char *BitRegs[8] = {&registers.B, &registers.C, &registers.D, &registers.E, &registers.H, &registers.L, NULL, &registers.A};
    struct opcode xor ;
    xor.inst = Opcode;
    unsigned char val;

    if (xor.r2 == 6)
    {
        val = ReadByte(registers.HL);
    }
    else
    {
        val = *BitRegs[xor.r2];
    }

    registers.A ^= val;

    // printf("%04x", xor.r1);
    if (registers.A == 0)
    {
        FLAG_SET(FLAG_ZERO);
    }
    else
    {
        FLAG_CLEAR(FLAG_ZERO);
    } // Z
    FLAG_CLEAR(FLAG_N);
    FLAG_CLEAR(FLAG_HALF);
    FLAG_CLEAR(FLAG_CARRY);

    CreateBox("after XOR");
}

void OR(unsigned char Opcode)
{

    unsigned char *BitRegs[8] = {&registers.B, &registers.C, &registers.D, &registers.E, &registers.H, &registers.L, NULL, &registers.A};
    struct opcode or ;
    or.inst = Opcode;
    unsigned char val;

    if (or.r2 == 6)
    {
        val = ReadByte(registers.HL);
    }
    else
    {
        val = *BitRegs[or.r2];
    }

    registers.A |= val;

    if (registers.A == 0)
    {
        FLAG_SET(FLAG_ZERO);
    }
    else
    {
        FLAG_CLEAR(FLAG_ZERO);
    }; // Z
    FLAG_CLEAR(FLAG_N);
    FLAG_CLEAR(FLAG_HALF);
    FLAG_CLEAR(FLAG_CARRY);
    CreateBox("after OR");
}

void CP(unsigned char Opcode)
{ // CP

    unsigned char *BitRegs[8] = {&registers.B, &registers.C, &registers.D, &registers.E, &registers.H, &registers.L, NULL, &registers.A};
    struct opcode cp;
    cp.inst = Opcode;

    unsigned char A = registers.A;
    unsigned char val;

    if (cp.r2 == 6)
    {
        val = ReadByte(registers.HL);
    }
    else
    {
        val = *BitRegs[cp.r2];
    }

    SUBC(&A, val, 0); // uses sub flags
    CreateBox("After CP Reg");

    // FLAG_SET((registers.A && registers.A) * 0x80); //Z
    // FLAG_SET(FLAG_N);
    // FLAG_CLEAR(FLAG_HALF);
    // FLAG_CLEAR(FLAG_CARRY); //LOOK

    // if (Opcode == 0xBF) {
    //     FLAG_SET(FLAG_ZERO);
    //     FLAG_SET(FLAG_N);
    // }
}

void SUB8(unsigned char Opcode, unsigned char operand)
{

    if (Opcode = 0xD6)
    {
        SUBC(&registers.A, operand, 0);
    }
    else
    {
        SUBC(&registers.A, operand, 1);
    }
}

void AND8(unsigned char Opcode, unsigned char operand)
{

    registers.A &= operand;

    if (registers.A == 0)
    {
        FLAG_SET(FLAG_ZERO);
    }
    else
    {
        FLAG_CLEAR(FLAG_ZERO);
    } // Z
    FLAG_CLEAR(FLAG_N);
    FLAG_SET(FLAG_HALF);
    FLAG_CLEAR(FLAG_CARRY);

    CreateBox("After AND8");
}

void XOR8(unsigned char Opcode, unsigned char operand)
{

    registers.A ^= operand;

    if (registers.A == 0)
    {
        FLAG_SET(FLAG_ZERO);
    }
    else
    {
        FLAG_CLEAR(FLAG_ZERO);
    } // Z
    FLAG_CLEAR(FLAG_N);
    FLAG_CLEAR(FLAG_HALF);
    FLAG_CLEAR(FLAG_CARRY);
    CreateBox("After XOR8");
}

void OR8(unsigned char Opcode, unsigned char operand)
{

    registers.A |= operand;

    if (registers.A == 0)
    {
        FLAG_SET(FLAG_ZERO);
    }
    else
    {
        FLAG_CLEAR(FLAG_ZERO);
    } // Z
    FLAG_CLEAR(FLAG_N);
    FLAG_CLEAR(FLAG_HALF);
    FLAG_CLEAR(FLAG_CARRY);
    CreateBox("After OR8");
}

void CP8(unsigned char Opcode, unsigned char operand)
{

    unsigned char A = registers.A;
    SUBC(&A, operand, 0); // uses sub flags
    CreateBox("After CP8");
}

void load8bit(unsigned char Opcode, unsigned char operand)
{
    switch (Opcode)
    {
    case 0xE0:
        
        WriteByte(0xFF00 + operand, registers.A);
        break;
    case 0xF0:
        registers.A = ReadByte(0xFF00 + operand);
        break;
    case 0xE2:
        WriteByte(0xFF00 + registers.C, registers.A);
        break;
    case 0xF2:
        registers.A = ReadByte(0xFF00 + registers.C);
        break;

    case 0x06:
        registers.B = operand;
        break;
    case 0x16:
        registers.D = operand;
        break;
    case 0x26:
        registers.H = operand;
        break;
    case 0x36:
        WriteByte(registers.HL, operand);
        break;

    case 0x0E: // ld (C) A
        registers.C = operand;
        break;
    case 0x1E:
        registers.E = operand;
        break;
    case 0x2E:
        registers.L = operand;
        break;
    case 0x3E:
        registers.A = operand;
        break;
    }
}

void load16bit(unsigned char Opcode, unsigned short operand)
{
    switch (Opcode)
    {
    case 0x01:
        registers.BC = operand;
        break;
    case 0x11:
        registers.DE = operand;
        break;
    case 0x21:
        registers.HL = operand;
        break;
    case 0x31:
        registers.SP = operand;
        break;
    case 0x08: // LD (a16) SP
        WriteShort(operand, registers.SP);
        break;
    case 0xEA:
        WriteByte(operand, registers.A);
        break;
    case 0xFA:
        registers.A = ReadByte(operand);
        break;
    case 0xF8:
        // Needs flags fo LD HL, SP + r8
        break;
    }
}

void loadRegB(unsigned char Opcode)
{
    // A: 7, B: 0, C: 1, D: 2, E: 3, H: 4, L: 5, (HL): 6
    unsigned char *BitRegs[8] = {&registers.B, &registers.C, &registers.D, &registers.E, &registers.H, &registers.L, NULL, &registers.A};
    struct opcode load;
    load.inst = Opcode;

    if (load.r1 == 0x06)
    {

        WriteByte(registers.HL, *BitRegs[load.r2]);
    }
    else if (load.r2 == 0x06)
    {
        *BitRegs[load.r1] = ReadByte(registers.HL);
    }
    else
    {
        *BitRegs[load.r1] = *BitRegs[load.r2];
    }
}

void loadRegS(unsigned char Opcode)
{
    switch (Opcode)
    {
    case 0x02:
        WriteByte(registers.BC, registers.A);
        break;
    case 0x12:
        WriteByte(registers.DE, registers.A);
        break;
    case 0x22:
        WriteByte(registers.HL, registers.A);
        registers.HL++;
        break;
    case 0x32:
        WriteByte(registers.HL, registers.A);
        registers.HL--;
        break;

    case 0x0A:
        registers.A = ReadByte(registers.BC);
        break;
    case 0x1A:
        registers.A = ReadByte(registers.DE);
        break;
    case 0x2A:
        registers.A = ReadByte(registers.HL);
        registers.HL++;
        break;
    case 0x3A:
        registers.A = ReadByte(registers.HL);
        registers.HL--;
        break;
    case 0xF9:
        registers.SP = registers.HL;
        break;
    }
}

void INCRegS(unsigned char Opcode)
{

    struct opcode Inc;
    Inc.inst = Opcode;
    char label[50];
    sprintf(label, "Increasing: 0x%04x by 1. r1 = 0x%04x", *topRowRegs[Inc.r1], Inc.r1);
    CreateBox(label);
    *topRowRegs[Inc.r1] += 1;
}

void INCRegB(unsigned char Opcode)
{
    switch (Opcode)
    {
    case 0x04:
        ADDC(&registers.B, 1, 2);
        break;
    case 0x14:
        ADDC(&registers.D, 1, 2);
        break;
    case 0x24:
        ADDC(&registers.H, 1, 2);
        break;
    case 0x34:
        // crazy solution
        unsigned char HLpointing = ReadByte(registers.HL);
        unsigned char *HLpointer = &HLpointing;

        ADDC(HLpointer, 1, 2);

        WriteByte(registers.HL, *HLpointer);

        break;

    case 0x0C:
        ADDC(&registers.C, 1, 2);
        break;
    case 0x1C:
        ADDC(&registers.E, 1, 2);
        break;
    case 0x2C:
        ADDC(&registers.L, 1, 2);
        break;
    case 0x3C:
        ADDC(&registers.A, 1, 2);
        break;
    }
}

void ADD(unsigned char Opcode)
{

    unsigned char *BitRegs[8] = {&registers.B, &registers.C, &registers.D, &registers.E, &registers.H, &registers.L, NULL, &registers.A};
    struct opcode add;
    add.inst = Opcode;
    unsigned char val;

    if (add.r2 == 0x06)
    {
        val = ReadByte(registers.HL);
    }
    else
    {
        val = *BitRegs[add.r2];
    }

    if (add.low > 0x07)
    {
        ADDC(&registers.A, val, 1);
        return;
    }
    else
    {
        ADDC(&registers.A, val, 0);
        return;
    }
}

void ADD8(unsigned char Opcode, unsigned char operand)
{
    if (Opcode == 0xC6)
    {
        ADDC(&registers.A, operand, 0);
        return;
    }
    else
    {
        ADDC(&registers.A, operand, 1);
        return;
    }
}

void SUB(unsigned char Opcode)
{

    unsigned char *BitRegs[8] = {&registers.B, &registers.C, &registers.D, &registers.E, &registers.H, &registers.L, NULL, &registers.A};
    struct opcode sub;
    sub.inst = Opcode;
    unsigned char val;
    if (sub.r2 == 0x06)
    {
        val = ReadByte(registers.HL);
    }
    else
    {
        val = *BitRegs[sub.r2];
    }

    if (sub.low > 0x07)
    {
        SUBC(&registers.A, val, 1);
        return;
    }
    else
    {
        SUBC(&registers.A, val, 0);
        return;
    }
}

void SUB16(unsigned short *A, unsigned char B)
{
    // don't edit Z for add HL, rr

    int diff = *A - B;

    FLAG_CLEAR(FLAG_ZERO);

    *A &= 0xFF, B &= 0xFF;
    if ((A - B) && 0x100)
    {
        FLAG_SET(FLAG_HALF);
    }
    else
    {
        FLAG_CLEAR(FLAG_HALF);
    }

    if ((diff < 0x0))
    {
        FLAG_SET(FLAG_CARRY);
    }
    else
    {
        FLAG_CLEAR(FLAG_CARRY);
    }

    *A = (unsigned short)diff;
}

void ADDC(unsigned char *destination, unsigned char val, unsigned char carry)
{
    //Carry 2 : dont impact Carry
    unsigned int sum = *destination + val;

    FLAG_CLEAR(FLAG_N);

    unsigned char A = *destination & 0x0F;
    unsigned char B = val & 0x0F;

    if ((A + B + (FLAG_ISCARRY && (carry == 1))) > 0x0F)
    {
        FLAG_SET(FLAG_HALF);
    } else FLAG_CLEAR(FLAG_HALF);

    sum += (FLAG_ISCARRY && (carry == 1));

    if(carry != 2){ //doesnt change on inc
            if (sum > 0xFF)
        {
            FLAG_SET(FLAG_CARRY);
        } else FLAG_CLEAR(FLAG_CARRY);
    }

    if ((unsigned char)sum == 0) // needs to be checked after carry operation if need
    {
        FLAG_SET(FLAG_ZERO);
    } else {
        FLAG_CLEAR(FLAG_ZERO);
    }

    *destination = (unsigned char)sum;
}

void DECRegB(unsigned char Opcode)
{
    switch (Opcode)
    {
    case 0x05:
        SUBC(&registers.B, 1, 2);
        break;
    case 0x15:
        SUBC(&registers.D, 1, 2);
        break;
    case 0x25:
        SUBC(&registers.H, 1, 2);
        break;
    case 0x35:
        unsigned char HLpointing = ReadByte(registers.HL);
        unsigned char *HLpointer = &HLpointing;

        SUBC(HLpointer, 1, 2);

        WriteByte(registers.HL, *HLpointer);

        break;

    case 0x0D:
        SUBC(&registers.C, 1, 2);
        break;
    case 0x1D:
        SUBC(&registers.E, 1, 2);
        break;
    case 0x2D:
        SUBC(&registers.L, 1, 2);
        break;
    case 0x3D:
        SUBC(&registers.A, 1, 2);
        break;
    }
}

void DECRegS(unsigned char Opcode)
{

    struct opcode Dec;
    Dec.inst = Opcode;
    char label[50];
    sprintf(label, "Decreasing: 0x%04x by 1. r1 = 0x%04x", *topRowRegs[Dec.r1 - 1], Dec.r1);
    CreateBox(label);
    *topRowRegs[Dec.r1 - 1] -= 1;
    // CHECK
}

void SUBC(unsigned char *destination, unsigned char val, unsigned char carry)
{

    CreateBox("In SUBC");
    // signs may or may nto work
    signed int diff = *destination - val;
    char label[50];
    sprintf(label, "diff: %d, *destination: 0x%04x, val: 0x%04x", diff, *destination, val);
    CreateBox(label);
    if (diff == 0)
    {
        FLAG_SET(FLAG_ZERO);
    }
    else
    {
        FLAG_CLEAR(FLAG_ZERO);
    } // Z

    FLAG_SET(FLAG_N);

    unsigned char A = *destination & 0x0F;
    unsigned char B = val & 0x0F;

    if ((A - B) & 0x10)
    {
        FLAG_SET(FLAG_HALF);
    }
    else
    {
        FLAG_CLEAR(FLAG_HALF);
    }

    // carry 2: dec, carry:0 no carry but CY, carry:1 carry and CY
    switch (carry)
    {
    case 0:
        if (diff < 0x0)
        {
            FLAG_SET(FLAG_CARRY);
        }
        else
        {
            FLAG_CLEAR(FLAG_CARRY);
        }
        *destination = (unsigned char)diff;
        break;
    case 1:
        if (diff < 0x0)
        {
            FLAG_SET(FLAG_CARRY);
        }
        else
        {
            FLAG_CLEAR(FLAG_CARRY);
        }
        *destination = (unsigned char)diff - FLAG_ISCARRY;
        break;
    case 2: // dec

        *destination = (unsigned char)diff;
        break;
    }
}

void RR(unsigned char Opcode)
{

    switch (Opcode)
    {
    case 0x7: // ROTATIONS
        ROTATE(&registers.A, 0, 1);
        FLAG_CLEAR(FLAG_ZERO);
        break;

    case 0x17:
        ROTATE(&registers.A, 1, 1);
        FLAG_CLEAR(FLAG_ZERO);
        break;

    case 0x0F:
        ROTATE(&registers.A, 0, 0);
        FLAG_CLEAR(FLAG_ZERO);
        break;

    case 0x1F:
        ROTATE(&registers.A, 1, 0); //RRA
        FLAG_CLEAR(FLAG_ZERO);
        break;
    }
}

void ROTATE(unsigned char *object, unsigned char carry, unsigned char direct)
{
    // direct[0: right, 1: left]
    // carry[0: none, 1:through carry]

    unsigned char temp;

    if (direct)
    {
        unsigned char C = ((*object & 0x80) > 0);
        if (carry) //C is coming from the byte
        {
            temp = C; //put last bit of byte into CY and CY into byte
            C = (FLAG_ISCARRY > 0);
            if(temp) {FLAG_SET(FLAG_CARRY);} else {FLAG_CLEAR(FLAG_CARRY);}
        }
        else
        {
            if(C) {FLAG_SET(FLAG_CARRY);} else {FLAG_CLEAR(FLAG_CARRY);}

        } // going through if 1
        *object = ((*object << 1) | (C));
    }
    else
    {
        unsigned char C = ((*object & 0x01) > 0);
        if (carry)
        {
            temp = C;
            C = (FLAG_ISCARRY > 0);
            if(temp) {FLAG_SET(FLAG_CARRY);} else {FLAG_CLEAR(FLAG_CARRY);}
        }
        else
        {
            if(C) {FLAG_SET(FLAG_CARRY);} else {FLAG_CLEAR(FLAG_CARRY);}
        } // going through if 1
        *object = ((*object >> 1) | (C << 7));
    }

    if (*object == 0)
    {
        FLAG_SET(FLAG_ZERO);

    } else {FLAG_CLEAR(FLAG_ZERO);}

    FLAG_CLEAR(FLAG_N);
    FLAG_CLEAR(FLAG_HALF);
    CreateBox("After Rotate");
}

void ADDSP(unsigned char Opcode, signed char operand)
{
    int go = 0;
    if (registers.SP == 0x000F) 
    {
        go = 1;
    }

    unsigned short temp = registers.SP;

    unsigned char D8 = (unsigned char)operand;
    signed char S8 = ((D8&127)-(D8&128));

    unsigned short SP = registers.SP + S8;

    
    if (S8 >= 0) 
    {

        if( ((registers.SP & 0xFF) + S8 ) > 0xFF ) {FLAG_SET(FLAG_CARRY);} else {FLAG_CLEAR(FLAG_CARRY);}
        if( ((registers.SP & 0xF) + (S8 & 0xF) ) > 0xF ) {FLAG_SET(FLAG_HALF);} else {FLAG_CLEAR(FLAG_HALF);}

    } 
    else 
    {
        if( (SP & 0xFF) <= (registers.SP & 0xFF) ) {FLAG_SET(FLAG_CARRY);} else {FLAG_CLEAR(FLAG_CARRY);}
        if( (SP & 0xF) <= (registers.SP & 0xF) ) {FLAG_SET(FLAG_HALF);} else {FLAG_CLEAR(FLAG_HALF);}
    }

    unsigned char tempFlags = registers.F;

    if (Opcode == 0xE8)
    {
        if (operand > 0)
        {
            ADDCS(&registers.SP, operand, 0);
        }
        else
        {
            SUB16(&registers.SP, (-1 * operand)); // Fix later
        }
    }
    else
    {
        if (operand > 0)
        {

            ADDCS(&registers.SP, operand, 0);
            registers.HL = registers.SP;
            registers.SP = temp;
        }
        else
        {

            SUB16(&registers.SP, (-1 * operand)); // Fix later
            registers.HL = registers.SP;
            registers.SP = temp;

        }

    }

    //calculation of Half and Carry from 

    registers.F = tempFlags; //return it to before

    FLAG_CLEAR(FLAG_ZERO);
    FLAG_CLEAR(FLAG_N);
    if(go)
    {
        printf("new SP: %04X S8: %d PC: %04X, F: %02X\n",registers.SP, S8, registers.PC, registers.F);
        
    }

}

void ADDHL(unsigned char Opcode)
{

    struct opcode AddHL;
    AddHL.inst = Opcode;
    unsigned temp = (registers.F & 0x80);
    ADDCS(&registers.HL, *topRowRegs[AddHL.r1 - 1], 0);
    if(temp) {FLAG_SET(FLAG_ZERO);} else {FLAG_CLEAR(FLAG_ZERO);} //do no change unto Z.

}

void ADDCS(unsigned short *destination, unsigned short val, unsigned char carry)
{

    unsigned int sum = *destination + val;
    if ((unsigned short)sum == 0)
    {
        FLAG_SET(FLAG_ZERO);
    } else FLAG_CLEAR(FLAG_ZERO);

    FLAG_CLEAR(FLAG_N);

    unsigned short A = *destination & 0x0FFF;
    unsigned short B = val & 0x0FFF;

    if ((A + B + (FLAG_ISCARRY && (carry == 1))) > 0xFFF)
    {
        FLAG_SET(FLAG_HALF);
    } else FLAG_CLEAR(FLAG_HALF);
    if (sum > 0xFFFF)
    {
        FLAG_SET(FLAG_CARRY);
    } else FLAG_CLEAR(FLAG_CARRY);

    sum += (FLAG_ISCARRY && (carry == 1));

    *destination = (unsigned short)sum;
}

void JUMPHL(void)
{
    char label[50];
    sprintf(label, "After JMPHL, prev:%04x", registers.PC);
    registers.PC = registers.HL;
    CreateBox(label);
}

void JUMP8(unsigned char Opcode, signed char offset)
{

    // NZ
    if (Opcode == 0x20 && FLAG_ISSET(FLAG_ZERO))
    {
        CreateBox("test fail");
        currentcycles--;
        return;
    } // subtract one cycle
    // NC
    if (Opcode == 0x30 && FLAG_ISSET(FLAG_CARRY))
    {
        CreateBox("test fail");
        currentcycles--;
        return;
    }
    // Z
    if (Opcode == 0x28 && !FLAG_ISSET(FLAG_ZERO))
    {
        CreateBox("test fail");
        currentcycles--;
        return;
    }
    // C
    if (Opcode == 0x38 && !FLAG_ISSET(FLAG_ZERO))
    {
        CreateBox("test fail");
        currentcycles--;
        return;
    }

    char label[50];
    sprintf(label, "After JMP8, prev:%04x", registers.PC);
    registers.PC += offset;
    CreateBox(label);
}

void JUMP16(unsigned char Opcode, unsigned short Address)
{

    // NZ
    if (Opcode == 0xC2 && FLAG_ISSET(FLAG_ZERO))
    {
        CreateBox("test fail");
        currentcycles--;
        return;
    } // subtract one cycle
    // NC
    if (Opcode == 0xD2 && FLAG_ISSET(FLAG_CARRY))
    {
        CreateBox("test fail");
        currentcycles--;
        return;
    }
    // Z
    if (Opcode == 0xCA && !FLAG_ISSET(FLAG_ZERO))
    {
        CreateBox("test fail");
        currentcycles--;
        return;
    }
    // C
    if (Opcode == 0xDA && !FLAG_ISSET(FLAG_ZERO))
    {
        CreateBox("test fail");
        currentcycles--;
        return;
    }

    char label[50];
    sprintf(label, "After JMP16, prev:%04x", registers.PC);
    registers.PC = Address;
    CreateBox(label);
}

void CALL(unsigned char Opcode, unsigned short Address)
{

    // NZ
    if (Opcode == 0xC4 && FLAG_ISSET(FLAG_ZERO))
    {
        CreateBox("test fail");
        currentcycles -= 3;
        return;
    } // subtract one cycle
    // NC
    if (Opcode == 0xD4 && FLAG_ISSET(FLAG_CARRY))
    {
        CreateBox("test fail");
        currentcycles -= 3;
        return;
    }
    // Z
    if (Opcode == 0xCC && !FLAG_ISSET(FLAG_ZERO))
    {
        CreateBox("test fail");
        currentcycles -= 3;
        return;
    }
    // C
    if (Opcode == 0xDC && !FLAG_ISSET(FLAG_ZERO))
    {
        CreateBox("test fail");
        currentcycles -= 3;
        return;
    }

    WriteStack(registers.PC);
    registers.PC = Address;
}

void RET(unsigned char Opcode)
{
    switch (Opcode){
        case 0xC0:
            if(!FLAG_ISZERO){

                registers.PC = ReadStack();

            } else currentcycles -= 3;
            break;
        case 0xD0:
            if(!FLAG_ISCARRY){

                registers.PC = ReadStack();

            } else currentcycles -= 3;
            break;
        case 0xC8:
            if(FLAG_ISZERO){

                registers.PC = ReadStack();

            } else currentcycles -= 3;
            break;
        case 0xD8:
            if(FLAG_ISCARRY){

                registers.PC = ReadStack();

            } else currentcycles -= 3;
            break;
        case 0xC9:

            registers.PC = ReadStack();

    }

}

void RETI(unsigned char Opcode)
{
    interrupt.master = 1;
    RET(Opcode);
}

void RST(unsigned char Opcode)
{

    struct opcode rst;
    rst.inst = Opcode;
    unsigned char RSTAddr[8] = {0x00, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38}; // Goes straight to these positions
    CALL(0x00, RSTAddr[rst.r1]);
}

void POP(unsigned char Opcode)
{

        unsigned short *botRowRegs[7] = {&registers.BC, NULL, &registers.DE, NULL, &registers.HL, NULL, &registers.AF};
    struct opcode pop;
    pop.inst = Opcode;
    // if(pop.r1 == 0x110) {
    //     registers.AF = ReadStack();
    //     registers.F &= 0xF0;
    //     return;
    // }

    *botRowRegs[pop.r1] = ReadStack();
    registers.F &= 0xF0;
    //clear all the lower of F

    
}

void PUSH(unsigned char Opcode)
{
    unsigned short *botRowRegs[7] = {&registers.BC, NULL, &registers.DE, NULL, &registers.HL, NULL, &registers.AF};
    struct opcode push;
    push.inst = Opcode;

    // if(push.r1 == 0x110) {
    //     WriteStack(registers.AF); //special case for AF not being in toprowRegs
    //     return;
    // }

    WriteStack(*botRowRegs[push.r1]);
}

void CB(unsigned char Opcode, unsigned char operand)
{

    struct opcode CB;
    CB.inst = operand;

    unsigned char direction;
    unsigned char *BitRegs[8] = {&registers.B, &registers.C, &registers.D, &registers.E, &registers.H, &registers.L, NULL, &registers.A};

    if (CB.low < 0x08)
    {
        direction = 1;
    }
    else
    {
        direction = 0;
    }

    unsigned char Bitnum;
    if (CB.high > 0x3 && CB.high < 0x8)
    {
        Bitnum = (CB.inst - 0x4) / 8;
    }
    else if (CB.high > 0x7 && CB.high < 0xC)
    {
        Bitnum = (CB.inst - 0x8) / 8;
    }
    else if (CB.high > 0xB)
    {
        Bitnum = (CB.inst - 0xC) / 8;
    }

    if (CB.r2 == 0x06)
    {
        unsigned char HLpointing = ReadByte(registers.HL);
        unsigned char *HLpointer = &HLpointing;

        switch (CB.high)
        {
        case 0x0:
            ROTATE(HLpointer, 0, direction);
            break;
        case 0x1:
            ROTATE(HLpointer, 1, direction);
            break;
        case 0x3:
            SHIFT(*HLpointer, 0, direction);
        case 0x4:
        case 0x5:
        case 0x6:
        case 0x7: // BIT

            BIT(Bitnum, *HLpointer, 0);
            break;

        case 0x8:
        case 0x9:
        case 0xA:
        case 0xB: // RES

            *HLpointer = BIT(Bitnum, *HLpointer, 2);
            break;

        case 0xC:
        case 0xD:
        case 0xE:
        case 0xF: // SET
            *HLpointer = BIT(Bitnum, *HLpointer, 1);
            break;
        }
        WriteByte(registers.HL, *HLpointer);
        return;
    }

    switch (CB.high)
    {

    case 0x0: // RLC, RRC
        ROTATE(BitRegs[CB.r2], 0, direction);
        break;

    case 0x1: // wrap THROUGH CY
        ROTATE(BitRegs[CB.r2], 1, direction);
        break;

    case 0x2: // SLA SRA
        SHIFT(*BitRegs[CB.r2], 0, direction);
        break;

    case 0x3: // SWAP, SRL

        if (CB.low < 0x08)
        { // SWAP
            struct opcode RegSwap;
            RegSwap.inst = *BitRegs[CB.r2];
            *BitRegs[CB.r2] = ((RegSwap.low << 4) + (RegSwap.high));
            return;
        }
        else
        { // SHIFT

            *BitRegs[CB.r2] = SHIFT(*BitRegs[CB.r2], 1, 0);
            return;
        }

    case 0x4:
    case 0x5:
    case 0x6:
    case 0x7: // BIT

        BIT(Bitnum, *BitRegs[CB.r2], 0);
        break;

    case 0x8:
    case 0x9:
    case 0xA:
    case 0xB: // RES

        *BitRegs[CB.r2] = BIT(Bitnum, *BitRegs[CB.r2], 2);
        break;

    case 0xC:
    case 0xD:
    case 0xE:
    case 0xF: // SET
        *BitRegs[CB.r2] = BIT(Bitnum, *BitRegs[CB.r2], 1);
        break;
    }


}

unsigned char SHIFT(unsigned char X, unsigned char logical, unsigned char direct)
{
    // direct[0: right, 1: left]
    //  logic/arithmetic[0: arithmetic, 1:logical]

    unsigned char C;

    if (direct)
    { // left
        C = ((X & 0x80) && (X & 0x80));

        if (!C)
        {
            FLAG_CLEAR(FLAG_CARRY);
        }
        else
        {
            FLAG_SET(FLAG_CARRY);
        }
        X <<= 1;
    }
    else
    { // right

        C = ((X & 0x01) && (X & 0x01));

        if (!C)
        {
            FLAG_CLEAR(FLAG_CARRY);
        }
        else
        {
            FLAG_SET(FLAG_CARRY);
        }

        if (!logical)
        {
            C = (X & 0x08);
        }
        else
        {
            C = 0;
        } // changes from 0 if arthmetic
        X = ((X >> 1) + C);
    }

    if(X == 0) {FLAG_SET(FLAG_ZERO);} else {FLAG_CLEAR(FLAG_ZERO);}
    FLAG_CLEAR(FLAG_N); // LOOK
    FLAG_CLEAR(FLAG_HALF);
    return X;
}

unsigned char BIT(unsigned char bit, unsigned char byte, unsigned char OP)
{

    // OP(operation): 0 = bit, 1 = set, 2 = res
    // No need for clocks here
    //  xx b r [r(HL) = 110]

    // sending pointer
    switch (OP)
    {
    case 0:
        byte &= (0x01 << bit); // Moves over until it reaches the num and ANDs it
        byte >>= bit;

        if(byte == 0) {FLAG_SET(FLAG_ZERO);} else {FLAG_CLEAR(FLAG_ZERO);}
        FLAG_CLEAR(FLAG_N); // LOOK
        FLAG_SET(FLAG_HALF);
        break;

    case 1:

        byte |= (0x01 << bit);
        break;

    case 2:
        byte &= ~(0x01 << bit);
        break;
    }
    return byte;
}

void Reset(void)
{

    registers.A = 0x01;
    registers.F = 0xB0; // If the header checksum is $00, then the carry and half-carry flags are clear; otherwise, they are both set.
    registers.B = 0x00; // for the bmg pokemon red start
    registers.C = 0x13;
    registers.D = 0x00;
    registers.E = 0xD8;
    registers.H = 0x01;
    registers.L = 0x4D;
    registers.SP = 0xfffe;
    registers.PC = 0x100;

    interrupt.master = 0;
    interrupt.enable = 0;
    interrupt.flag = 0x01;

    timer.TIMA = 0;
    timer.TMA = 0;
    timer.TAC = 0;
    
    // lcd.LY = 50;

    // [$FF10] = $80 ; NR10
    // [$FF11] = $BF ; NR11
    // [$FF12] = $F3 ; NR12
    // [$FF14] = $BF ; NR14
    // [$FF16] = $3F ; NR21
    // [$FF17] = $00 ; NR22
    // [$FF19] = $BF ; NR24
    // [$FF1A] = $7F ; NR30
    // [$FF1B] = $FF ; NR31
    // [$FF1C] = $9F ; NR32
    // [$FF1E] = $BF ; NR33
    // [$FF20] = $FF ; NR41
    // [$FF21] = $00 ; NR42
    // [$FF22] = $00 ; NR43
    // [$FF23] = $BF ; NR30
    // [$FF24] = $77 ; NR50
    // [$FF25] = $F3 ; NR51
    // [$FF26] = $F1-GB, $F0-SGB ; NR52
    // AUDIO

    // lcd.control = 0x90;
    // lcd.status = 0x85;
    // lcd.SCY = 0x00;
    // lcd.SCX = 0;
    // lcd.LYC = 0;

    // WriteByte(0xFF47, 0xFC); // BGP = 0xFF;
    // WriteByte(0xFF48, 0xFF);
    // WriteByte(0xFF49, 0xFF);

    // lcd.WY = 0;
    // lcd.WX = 0;
}

void RequestInterrupt(int val)
{
    // printf("Request with Value: %d\n", val);
    switch (val)
    {
    case 0:
        interrupt.FVBlank = 1;
        break;
    case 1:
        interrupt.FLCD = 1;
        break;
    case 2:
        interrupt.FTimer = 1;
        break;
    case 3:
        interrupt.FSerial = 1;
        break;
    case 4:
        interrupt.FJoypad = 1;
        break;
    }
    // printf("Request Interrupt assigned, IF: %02X val: %02X\n", interrupt.flag, val);

}

void HandleInterrupt(void)
{

    if(interrupt.master == 0) {
        return;
    }

    // printf("Handling Interrupt");
    unsigned short InterruptAddress[5] = {0x0040, 0x0048, 0x0050, 0x0058, 0x0060};

    unsigned char priority;
    unsigned char currentFlag = 0;

    if (interrupt.FVBlank)
    {
        currentFlag = 0x01;
        priority = 0;
    }
    else if (interrupt.FLCD)
    {
        currentFlag = 0x02;
        priority = 1;
    }
    else if (interrupt.FTimer)
    {
        currentFlag = 0x04;
        priority = 2;
    }
    else if (interrupt.FSerial)
    {
        currentFlag = 0x08;
        priority = 3;
    }
    else if(interrupt.FJoypad)
    {
        currentFlag = 0x10;
        priority = 4;
    }

    if (interrupt.enable & currentFlag)
    {

        interrupt.master = 0;
        interrupt.flag &= ~(currentFlag);
        interrupt.enable &= ~(currentFlag);

        CALL(0x00, InterruptAddress[priority]);

    } // interrupt call
}

void Update(void)
{ // gpu step

    const int MAXCYCLES = 69905;
    currentcycles = 0;
    

    while (currentcycles < MAXCYCLES)
    {

        int cycles = realtimeDebug();
        currentcycles += cycles;
        // UpdateGraphics(cycles);
        
        UpdateTiming(cycles);
        HandleInterrupt();

    }

    frame++;
    // instSkips = 0;
    // CreateBox("frame");
    // instSkips = 1;
}

void UpdateTiming(int cycles)
{

    UpdateDivider(cycles);

    if (IsClockEnabled())
    {

        m_timercounter -= (cycles * 4);

        if (m_timercounter <= 0)
        {

            m_timercounter = CLOCKSPEED / GetFrequency();

        
            if (timer.TIMA == 255)
            {
                timer.TIMA = timer.TMA;

                RequestInterrupt(2);
            }
            else
            {
                timer.TIMA++;
            }
        }
    }
}

void UpdateDivider(int cycles)
{

    m_dividercounter -= cycles;

    if (m_dividercounter <= 0)
    {

        m_dividercounter = 256;
        timer.DIV++;
    }
}

int IsClockEnabled()
{
    return (timer.TAC & 0x04);
}

int GetFrequency()
{
    int frequency;
    frequency = (timer.TAC & 0x03);
    switch (frequency)
    {
    case 0:
        return 4096;
    case 1:
        return 262144;
    case 2:
        return 65536;
    case 3:
        return 16384;
    }
}