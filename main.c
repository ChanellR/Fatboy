#include <stdio.h>

#include "header.h"
#include "memory.h"

struct timer timer;
struct interrupt interrupt;
struct registers registers;
unsigned char stopped;


void Reset (void) {

    registers.A = 0x01;
	registers.F = 0xb0; //If the header checksum is $00, then the carry and half-carry flags are clear; otherwise, they are both set.
	registers.B = 0x00;
	registers.C = 0x13;
	registers.D = 0x00;
	registers.E = 0xd8;
	registers.H = 0x01;
	registers.L = 0x4d;
	registers.SP = 0xfffe;
	registers.PC = 0x100;
	
	interrupt.master = 1;
	interrupt.enable = 0;
	interrupt.flag = 0;

}

const struct Instruction instructions[257] = {

    //-3: word LD, -2: byte LD, -1: register specific, 0: none, 1:byte, 2: word

    {0, nop, 1},   //0x00 
    {-3, load16bit, 3}, //0x01
    {-1, loadRegS, 2},  //0x02
    {-1, INCRegS, 2},   //0x03
    {-1, INCRegB, 1},   //0x04   
    {-1, DECRegB, 1},   //0x05
    {-2, load8bit, 2},   //0x06
    {-1, RR, 1},   //0x07
    {-3, load16bit, 5},   //0x08
    {-1, ADDHL, 2},   //0x09
    {-1, loadRegB, 2}, //0x0a
    {-1, DECRegS, 2},  //0x0b
    {-1, INCRegB, 1},   //0x0c
    {-1, DECRegB, 1},   //0x0d
    {-2, load8bit, 2},   //0x0e
    {-1, RR, 1},   //0x0f

    {0, STOP, 2},   //0x00 
    {-3, load16bit, 3}, //0x01
    {-1, loadRegS, 2},  //0x02
    {-1, INCRegS, 2},   //0x03
    {-1, INCRegB, 1},   //0x04   
    {-1, DECRegB, 1},   //0x05
    {-2, load8bit, 2},   //0x06
    {-1, RR, 1},   //0x07
    {-2, JUMP8, 3},   //0x08
    {-1, ADDHL, 2},   //0x09
    {-1, loadRegB, 2}, //0x0a
    {-1, DECRegS, 2},  //0x0b
    {-1, INCRegB, 1},   //0x0c
    {-1, DECRegB, 1},   //0x0d
    {-2, load8bit, 2},   //0x0e
    {-1, RR, 1},   //0x0f

    {-2, JUMP8, 3},   //0x00 
    {-3, load16bit, 3}, //0x01
    {-1, loadRegS, 2},  //0x02
    {-1, INCRegS, 2},   //0x03
    {-1, INCRegB, 1},   //0x04   
    {-1, DECRegB, 1},   //0x05
    {-2, load8bit, 2},   //0x06
    {0, DAA, 1},   //0x07
    {-2, JUMP8, 3},   //0x08
    {-1, ADDHL, 2},   //0x09
    {-1, loadRegB, 2}, //0x0a
    {-1, DECRegS, 2},  //0x0b
    {-1, INCRegB, 1},   //0x0c
    {-1, DECRegB, 1},   //0x0d
    {-2, load8bit, 2},   //0x0e
    {0, CPL, 1},   //0x0f

    {-2, JUMP8, 3},   //0x00 
    {-3, load16bit, 3}, //0x01
    {-1, loadRegS, 2},  //0x02
    {-1, INCRegS, 2},   //0x03
    {-1, INCRegB, 1},   //0x04   
    {-1, DECRegB, 1},   //0x05
    {-2, load8bit, 2},   //0x06
    {0, SCF, 1},   //0x07
    {-2, JUMP8, 3},   //0x08
    {-1, ADDHL, 2},   //0x09
    {-1, loadRegB, 2}, //0x0a
    {-1, DECRegS, 2},  //0x0b
    {-1, INCRegB, 1},   //0x0c
    {-1, DECRegB, 1},   //0x0d
    {-2, load8bit, 2},   //0x0e
    {0, CCF, 1},   //0x0f

    {-1, loadRegB, 1}, //0x00
    {-1, loadRegB, 1}, //0x01
    {-1, loadRegB, 1}, //0x02
    {-1, loadRegB, 1}, //0x03
    {-1, loadRegB, 1}, //0x04
    {-1, loadRegB, 1}, //0x05
    {-1, loadRegB, 1}, //0x06
    {-1, loadRegB, 1}, //0x07
    {-1, loadRegB, 1}, //0x08
    {-1, loadRegB, 1}, //0x09
    {-1, loadRegB, 1}, //0x0A
    {-1, loadRegB, 1}, //0x0B
    {-1, loadRegB, 1}, //0x0C
    {-1, loadRegB, 1}, //0x0D
    {-1, loadRegB, 1}, //0x0E
    {-1, loadRegB, 1}, //0x0F

    {-1, loadRegB, 1}, //0x00
    {-1, loadRegB, 1}, //0x01
    {-1, loadRegB, 1}, //0x02
    {-1, loadRegB, 1}, //0x03
    {-1, loadRegB, 1}, //0x04
    {-1, loadRegB, 1}, //0x05
    {-1, loadRegB, 1}, //0x06
    {-1, loadRegB, 1}, //0x07
    {-1, loadRegB, 1}, //0x08
    {-1, loadRegB, 1}, //0x09
    {-1, loadRegB, 1}, //0x0A
    {-1, loadRegB, 1}, //0x0B
    {-1, loadRegB, 1}, //0x0C
    {-1, loadRegB, 1}, //0x0D
    {-1, loadRegB, 1}, //0x0E
    {-1, loadRegB, 1}, //0x0F

    {-1, loadRegB, 1}, //0x00
    {-1, loadRegB, 1}, //0x01
    {-1, loadRegB, 1}, //0x02
    {-1, loadRegB, 1}, //0x03
    {-1, loadRegB, 1}, //0x04
    {-1, loadRegB, 1}, //0x05
    {-1, loadRegB, 1}, //0x06
    {-1, loadRegB, 1}, //0x07
    {-1, loadRegB, 1}, //0x08
    {-1, loadRegB, 1}, //0x09
    {-1, loadRegB, 1}, //0x0A
    {-1, loadRegB, 1}, //0x0B
    {-1, loadRegB, 1}, //0x0C
    {-1, loadRegB, 1}, //0x0D
    {-1, loadRegB, 1}, //0x0E
    {-1, loadRegB, 1}, //0x0F

    {-1, loadRegB, 1}, //0x00
    {-1, loadRegB, 1}, //0x01
    {-1, loadRegB, 1}, //0x02
    {-1, loadRegB, 1}, //0x03
    {-1, loadRegB, 1}, //0x04
    {-1, loadRegB, 1}, //0x05
    {0, HALT, 1}, //0x06
    {-1, loadRegB, 1}, //0x07
    {-1, loadRegB, 1}, //0x08
    {-1, loadRegB, 1}, //0x09
    {-1, loadRegB, 1}, //0x0A
    {-1, loadRegB, 1}, //0x0B
    {-1, loadRegB, 1}, //0x0C
    {-1, loadRegB, 1}, //0x0D
    {-1, loadRegB, 1}, //0x0E
    {-1, loadRegB, 1}, //0x0F

    {-1, ADD, 1}, //0x00
    {-1, ADD, 1}, //0x01
    {-1, ADD, 1}, //0x02
    {-1, ADD, 1}, //0x03
    {-1, ADD, 1}, //0x04
    {-1, ADD, 1}, //0x05
    {-1, ADD, 1}, //0x06
    {-1, ADD, 1}, //0x07
    {-1, ADD, 1}, //0x08
    {-1, ADD, 1}, //0x09
    {-1, ADD, 1}, //0x0A
    {-1, ADD, 1}, //0x0B
    {-1, ADD, 1}, //0x0C
    {-1, ADD, 1}, //0x0D
    {-1, ADD, 1}, //0x0E
    {-1, ADD, 1}, //0x0F
    {-1, ADD, 1}, //0x0G

    {-1, SUB, 1}, //0x00
    {-1, SUB, 1}, //0x01
    {-1, SUB, 1}, //0x02
    {-1, SUB, 1}, //0x03
    {-1, SUB, 1}, //0x04
    {-1, SUB, 1}, //0x05
    {-1, SUB, 1}, //0x06
    {-1, SUB, 1}, //0x07
    {-1, SUB, 1}, //0x08
    {-1, SUB, 1}, //0x09
    {-1, SUB, 1}, //0x0A
    {-1, SUB, 1}, //0x0B
    {-1, SUB, 1}, //0x0C
    {-1, SUB, 1}, //0x0D
    {-1, SUB, 1}, //0x0E
    {-1, SUB, 1}, //0x0F

    {-1, AND, 1}, //0x0
    {-1, AND, 1},
    {-1, AND, 1},
    {-1, AND, 1},
    {-1, AND, 1},
    {-1, AND, 1},
    {-1, AND, 1},
    {-1, AND, 1},

    {-1, XOR, 1}, //0x08
    {-1, XOR, 1},
    {-1, XOR, 1},
    {-1, XOR, 1},
    {-1, XOR, 1},
    {-1, XOR, 1},
    {-1, XOR, 1},
    {-1, XOR, 1},

    {-1, OR, 1}, //0x00
    {-1, OR, 1},
    {-1, OR, 1},
    {-1, OR, 1},
    {-1, OR, 1},
    {-1, OR, 1},
    {-1, OR, 1},
    {-1, OR, 1},

    {-1, CP, 1}, //0x08
    {-1, CP, 1},
    {-1, CP, 1},
    {-1, CP, 1},
    {-1, CP, 1},
    {-1, CP, 1},
    {-1, CP, 1},
    {-1, CP, 1},
///Cx
    {-1, RET, 5}, //0x00
    {-1, POP, 3},
    {-3, JUMP16, 4},
    {-3, JUMP16, 4},
    {-3, CALL, 6},
    {-1, PUSH, 4},
    {-2, ADD8, 2},
    {-1, RST, 4},

    {-1, RET, 5}, //0x08
    {-1, RET, 4},
    {-3, JUMP16, 4},
    {-2, CB, 2},
    {-3, CALL, 6},
    {-3, CALL, 6},
    {-2, ADD8, 1},
    {-1, RST, 4},
///Dx
    {-1, RET, 5}, //0x00
    {-1, POP, 3},
    {-3, JUMP16, 4},
    {0, nop, 1},
    {-3, CALL, 6},
    {-1, PUSH, 4},
    {-2, SUB8, 2},
    {-1, RST, 4},

    {-1, RET, 5}, //0x08
    {-1, RETI, 4},
    {-3, JUMP16, 4},
    {0, nop, 1},
    {-3, CALL, 6},
    {0, nop, 1},
    {-2, SUB8, 2},
    {-1, RST, 4},
//Ex
    {-1, load8bit, 3}, //0x00
    {-1, POP, 3},
    {-1, load8bit, 2},
    {0, nop, 1},
    {0, nop, 1},
    {-1, PUSH, 4},
    {-2, AND8, 2},
    {-1, RST, 4},

    {-4, ADDSP, 4}, //0x08
    {0, JUMPHL, 1},
    {-3, load16bit, 4},
    {0, nop, 1},
    {0, nop, 1},
    {0, nop, 1},
    {-2, XOR8, 2},
    {-1, RST, 4},
//Fx
    {-1, load8bit, 3}, //0x00
    {-1, POP, 3},
    {-3, load8bit, 2},
    {0, DI, 1},
    {0, nop, 1},
    {-1, PUSH, 4},
    {-2, OR8, 2},
    {-1, RST, 4},

    {-4, ADDSP, 3}, //0x08
    {-1, loadRegS, 2},
    {-3, load16bit, 4},
    {0, EI, 1},
    {0, nop, 1},
    {0, nop, 1},
    {0, CP8, 2},
    {-1, RST, 4}
    
};


int main (void) {
    //the asterisk is an empty function constructor, which calls the pointer function from instructions i suppose

    unsigned char instruction = ReadByte(registers.PC++);

    switch (instructions[instruction].operand_length){
        
        case -4:
            ((void (*)(unsigned char, signed char))instructions[instruction].function)(instruction, ReadByte(registers.PC++));
        case -3:
            ((void (*)(unsigned char, unsigned short))instructions[instruction].function)(instruction, ReadShort(registers.PC));
            registers.PC += 2;
            break;
        case -2:
            ((void (*)(unsigned char, unsigned char))instructions[instruction].function)(instruction, ReadByte(registers.PC++));
            break;
        case 0:
            ((void (*)(unsigned char))instructions[instruction].function)(instruction);
            break;
        case 1:
            ((void (*)(unsigned char))instructions[instruction].function)(ReadByte(registers.PC++));
            break;

    }

    //Handle timing 
    registers.HL = 0;
    registers.B = 4;

    // ((void (*)(unsigned char))instructions[2].function)(0x70);
    printf("%i", instructions[256].operand_length);

}





//BC: 000, DE: 010, HL: 100, SP: 110 r1
// B: 000, D: 010, H:100, (HL): 110 r1
// C: 001, E: 011, L: 101, A:111 
unsigned short *topRowRegs[7] = {&registers.BC, NULL, &registers.DE, NULL, &registers.HL, NULL, &registers.SP};
unsigned char *topRowRegsHigh[6] = {&registers.B, NULL, &registers.D, NULL, &registers.H, NULL}; //(HL) should be at end
unsigned char *topRowRegsHigh2[8] = {NULL, &registers.C, NULL, &registers.E, NULL, &registers.L, NULL, &registers.A};

void nop(void) {}

void STOP(void) {
    stopped = 0;
    //Reset, start at 0000h
    //input, start after instruction
}

void HALT (void) {
    stopped = 1;
    //Halted
}

void DAA (void) {
    // note: assumes a is a uint8_t and wraps from 0xff to 0
if (!FLAG_ISSET(FLAG_N)) {  // after an addition, adjust if (half-)carry occurred or if result is out of bounds
  if (FLAG_ISSET(FLAG_CARRY) || registers.A > 0x99) { registers.A += 0x60; FLAG_SET(FLAG_CARRY); }
  if (FLAG_ISSET(FLAG_HALF) || (registers.A & 0x0f) > 0x09) { registers.A += 0x6; }
} else {  // after a subtraction, only adjust if (half-)carry occurred
  if (FLAG_ISSET(FLAG_CARRY)) { registers.A -= 0x60; }
  if (FLAG_ISSET(FLAG_HALF)) { registers.A -= 0x6; }
}
// these flags are always updated
FLAG_SET((registers.A && registers.A) * 0x80); // the usual z flag
FLAG_CLEAR(FLAG_HALF); // h flag is always cleared

}


void CPL (void) {

    registers.A &= 0xFF;
}

void SCF (void) {
    
    FLAG_SET(FLAG_CARRY);
}

void CCF (void) {

    if(FLAG_ISSET(FLAG_CARRY)) {
        FLAG_CLEAR(FLAG_CARRY);
    } else {
        FLAG_SET(FLAG_CARRY);
    }
}

void DI (void) {
    interrupt.master = 0;
}

void EI (void) {
    interrupt.master = 0;
}

void AND (unsigned char Opcode) {

    unsigned char *BitRegs[8] = {&registers.B, &registers.C, &registers.D, &registers.E, &registers.H, &registers.L, &memory[registers.HL], &registers.A};
    struct opcode and;
    and.inst = Opcode;
    registers.A &= *BitRegs[and.r2];

    FLAG_SET((registers.A && registers.A) * 0x80); //Z
    FLAG_CLEAR(FLAG_N);
    FLAG_SET(FLAG_HALF);
    FLAG_CLEAR(FLAG_CARRY);

}

void XOR (unsigned char Opcode) {

    if(Opcode == 0xAF) {FLAG_SET(FLAG_ZERO);} return;

    unsigned char *BitRegs[8] = {&registers.B, &registers.C, &registers.D, &registers.E, &registers.H, &registers.L, &memory[registers.HL], &registers.A};
    struct opcode xor;
    xor.inst = Opcode;
    registers.A ^= *BitRegs[xor.r2];

    FLAG_SET((registers.A && registers.A) * 0x80); //Z
    FLAG_CLEAR(FLAG_N);
    FLAG_CLEAR(FLAG_HALF);
    FLAG_CLEAR(FLAG_CARRY);

}

void OR (unsigned char Opcode) {

    unsigned char *BitRegs[8] = {&registers.B, &registers.C, &registers.D, &registers.E, &registers.H, &registers.L, &memory[registers.HL], &registers.A};
    struct opcode or;
    or.inst = Opcode;
    registers.A |= *BitRegs[or.r2];

    FLAG_SET((registers.A && registers.A) * 0x80); //Z
    FLAG_CLEAR(FLAG_N);
    FLAG_CLEAR(FLAG_HALF);
    FLAG_CLEAR(FLAG_CARRY);
}

void CP (unsigned char Opcode) { //CP

    unsigned char *BitRegs[8] = {&registers.B, &registers.C, &registers.D, &registers.E, &registers.H, &registers.L, &memory[registers.HL], &registers.A};
    struct opcode xor;
    xor.inst = Opcode;
    registers.A = (registers.A == *BitRegs[xor.r2]);


    FLAG_SET((registers.A && registers.A) * 0x80); //Z
    FLAG_SET(FLAG_N);
    FLAG_CLEAR(FLAG_HALF);
    FLAG_CLEAR(FLAG_CARRY); //LOOK

    if (Opcode == 0xBF) {
        FLAG_SET(FLAG_ZERO);
        FLAG_SET(FLAG_N);
    }
}

void SUB8 (unsigned char Opcode, unsigned char operand) {

        if (Opcode = 0xD6) {
        SUBC(&registers.A, operand, 0);
    } else {
        SUBC(&registers.A, operand, 1);
    }
}


void AND8 (unsigned char Opcode, unsigned char operand) {

    registers.A &= operand;

    FLAG_SET((registers.A && registers.A) * 0x80); //Z
    FLAG_CLEAR(FLAG_N);
    FLAG_SET(FLAG_HALF);
    FLAG_CLEAR(FLAG_CARRY);

}

void XOR8 (unsigned char Opcode, unsigned char operand) {

    registers.A ^= operand;

    FLAG_SET((registers.A && registers.A) * 0x80); //Z
    FLAG_CLEAR(FLAG_N);
    FLAG_CLEAR(FLAG_HALF);
    FLAG_CLEAR(FLAG_CARRY);
}

void OR8 (unsigned char Opcode, unsigned char operand) {

    registers.A |= operand;

    FLAG_SET((registers.A && registers.A) * 0x80); //Z
    FLAG_CLEAR(FLAG_N);
    FLAG_CLEAR(FLAG_HALF);
    FLAG_CLEAR(FLAG_CARRY);
}


void CP8 (unsigned char Opcode, unsigned char operand) {

    registers.A = (registers.A == operand);


    FLAG_SET((registers.A && registers.A) * 0x80); //Z
    FLAG_SET(FLAG_N);
    FLAG_CLEAR(FLAG_HALF);
    FLAG_CLEAR(FLAG_CARRY); //LOOK

}


void load8bit(unsigned char Opcode, unsigned char operand){
    switch(Opcode){
        case 0xE0: //ld (C) A
            WriteByte( 0xFF00 + registers.C, registers.A );
            break;
        case 0xF0:
            registers.A = ReadByte( 0xFF00 + registers.C );
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
        
        case 0x0E:
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

void load16bit(unsigned char Opcode, unsigned short operand) {
    switch (Opcode){
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
        case 0x08: //LD (a16) SP
            WriteShort (operand, registers.SP);
            break;
        case 0xEA:
            WriteByte(operand, registers.A);
            break;
        case 0xFA:
            registers.A = ReadByte(operand);
            break;
        case 0xF8:
            //Needs flags fo LD HL, SP + r8
            break;

    }
}

void loadRegB(unsigned char Opcode) {
    //A: 7, B: 0, C: 1, D: 2, E: 3, H: 4, L: 5, (HL): 6
    unsigned char *BitRegs[8] = {&registers.B, &registers.C, &registers.D, &registers.E, &registers.H, &registers.L, &memory[registers.HL], &registers.A};
    struct opcode load;
    load.inst = Opcode;
    *BitRegs[load.r1] = *BitRegs[load.r2];

}

void loadRegS (unsigned char Opcode) {
    switch (Opcode) {
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

void INCRegS(unsigned char Opcode) {
    
    struct opcode Inc;
    Inc.inst = Opcode;

    *topRowRegs[Inc.r1]++;
}

void INCRegB (unsigned char Opcode) {
    switch(Opcode){
        case 0x04:
            ADDC(&registers.B, 1, 0);
            break;
        case 0x14:
            ADDC(&registers.D, 1, 0);
            break;
        case 0x24:
            ADDC(&registers.H, 1, 0);
            break;
        case 0x34:
            ADDC(&memory[registers.HL], 1, 0);
            break;

        case 0x0C:
            ADDC(&registers.C, 1, 0);
            break;
        case 0x1C:
            ADDC(&registers.E, 1, 0);
            break;
        case 0x2C:
            ADDC(&registers.L, 1, 0);
            break;
        case 0x3C:
            ADDC(&registers.A, 1, 0);
            break;

    }
}

void ADD (unsigned char Opcode){

    unsigned char *BitRegs[8] = {&registers.B, &registers.C, &registers.D, &registers.E, &registers.H, &registers.L, &memory[registers.HL], &registers.A};
    struct opcode add;
    add.inst = Opcode;
    if (add.low > 0x07) {
        ADDC(&registers.A, *BitRegs[add.r2], 1);
        return;
    } else {
        ADDC(&registers.A, *BitRegs[add.r2], 0);
        return;
    }   

}

void ADD8 (unsigned char Opcode, unsigned char operand) {
    if(Opcode == 0xC6) {
        ADDC(&registers.A, operand, 0);
        return;   
    } else {
        ADDC(&registers.A, operand, 1);
        return; 
    }
}

void SUB (unsigned char Opcode) {
    
    unsigned char *BitRegs[8] = {&registers.B, &registers.C, &registers.D, &registers.E, &registers.H, &registers.L, &memory[registers.HL], &registers.A};
    struct opcode sub;
    sub.inst = Opcode;
    if (sub.low > 0x07) {
        SUBC(&registers.A, *BitRegs[sub.r2], 1);
        return;
    } else {
        SUBC(&registers.A, *BitRegs[sub.r2], 0);
        return;
    }   
}

void SUB16 (unsigned short *A, unsigned char B){
    //don't edit Z for add HL, rr
    
    int diff = *A - B;

    FLAG_CLEAR(FLAG_ZERO);

    *A &= 0xFF, B &= 0xFF;
    if ((A - B) && 0x100) {FLAG_SET(FLAG_HALF);} else {FLAG_CLEAR(FLAG_HALF);}

    if((diff < 0x0)) {FLAG_SET(FLAG_CARRY);} else {FLAG_CLEAR(FLAG_CARRY);}

    *A = (unsigned char) diff;
    
}

void ADDC (unsigned char *destination, unsigned char val, unsigned char carry){

    unsigned int sum = *destination + val;
    if (sum == 0) {FLAG_SET(FLAG_ZERO);} //Z
    FLAG_CLEAR(FLAG_N);

    unsigned char A = *destination & 0x0F; 
    unsigned char B = val & 0x0F;

    if ((A + B) > 0x0F) {FLAG_SET(FLAG_HALF);}
    if( sum > 0xFF ) {FLAG_SET(FLAG_CARRY);}

    sum += (FLAG_ISCARRY & carry);

    *destination = (unsigned char)sum;

}

void DECRegB (unsigned char Opcode) {
    switch(Opcode){
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
            SUBC(&memory[registers.HL], 1, 2);
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

void DECRegS (unsigned char Opcode) {
   
    struct opcode Dec;
    Dec.inst = Opcode;

    *topRowRegs[Dec.r1-3]--;
    //CHECK
}

void SUBC (unsigned char *destination, unsigned char val, unsigned char carry){

    //signs may or may nto work
    signed int diff = *destination - val;
    if (diff == 0) {FLAG_SET(FLAG_ZERO);} //Z
    FLAG_SET(FLAG_N);

    unsigned char A = *destination & 0x0F; 
    unsigned char B = val & 0x0F;

    if((A - B) & 0x10){FLAG_SET(FLAG_HALF);}

    //carry 2: dec, carry:0 no carry but CY, carry:1 carry and CY
    switch(carry){
        case 0:
            if(diff < 0x0) {FLAG_SET(FLAG_CARRY);}
            *destination = (unsigned char)diff;
            break;
        case 1:
            if(diff < 0x0) {FLAG_SET(FLAG_CARRY);}
            *destination = (unsigned char)diff - FLAG_ISCARRY;
            break;
        case 2: //dec
            *destination = (unsigned char)diff;
            break;
    }

}

void RR (unsigned char Opcode) {

    switch (Opcode){
        case 0x7: //ROTATIONS
            ROTATE(&registers.A, 0, 1);
            break;

        case 0x17:
            ROTATE(&registers.A, 1, 1);
            break;

        case 0x27:
            ROTATE(&registers.A, 0, 1);
            break;

        case 0x37:
            ROTATE(&registers.A, 1, 0);
            break;
    }
}

void ROTATE (unsigned char *object, unsigned char carry, unsigned char direct){
    //direct[0: right, 1: left]
    //carry[0: none, 1:through carry]

    if(direct){
        unsigned char C = (*object & 0x80 || *object & 0x80);
        if(carry) {unsigned char temp = C; C = (FLAG_ISCARRY); FLAG_SET(temp << 4);} else {FLAG_SET(C << 4);} //going through if 1
        *object = (*object<<1) + (C);
    } else {
        unsigned char C = (*object & 0x01 || *object & 0x01);
        if(carry) {unsigned char temp = C; C = (FLAG_ISCARRY); FLAG_SET(temp << 4);} else {FLAG_SET(C << 4);} //going through if 1
        *object = (*object>>1) + (C<<7);
    }

    if (*object == 0) {FLAG_SET(FLAG_ZERO);}
    FLAG_CLEAR(FLAG_N);
    FLAG_CLEAR(FLAG_HALF);

}

void ADDSP (unsigned char Opcode, signed char operand) {

    unsigned char temp = registers.SP;

    if (Opcode == 0xE8) {
        if(operand > 0) {
            ADDCS(&registers.SP, operand, 0);
        } else {
            SUB16(&registers.SP, (signed char) operand); //Fix later
        }
        
    } else {
        if(operand > 0) {
            
            ADDCS(&registers.SP, operand, 0);
            registers.HL = registers.SP;
            registers.SP = temp;

        } else {
            SUB16(&registers.SP, (signed char) operand); //Fix later
            registers.HL = registers.SP;
            registers.SP = temp;
    }
}
}

void ADDHL(unsigned char Opcode) {

    struct opcode AddHL;
    AddHL.inst = Opcode;

    ADDCS(&registers.HL, *topRowRegs[AddHL.r1-1], 0);
    
}

void ADDCS (unsigned short *destination, unsigned short val, unsigned char carry){


    unsigned int sum = *destination + val;
    if (sum == 0) {FLAG_SET(FLAG_ZERO);} //Z
    FLAG_CLEAR(FLAG_N);

    unsigned char A = *destination & 0xFF; 
    unsigned char B = val & 0xFF;

    if ((A + B) > 0xFF) {FLAG_SET(FLAG_HALF);}
    if( sum > 0xFFFF ) {FLAG_SET(FLAG_CARRY);}

    sum += (FLAG_ISCARRY & carry);

    *destination = (unsigned char)sum;

}

void JUMPHL (void) {
    registers.PC = registers.HL;
}

void JUMP8 (unsigned char Opcode, signed char offset) {

    //NZ
    if(Opcode == 0x20 && FLAG_ISSET(FLAG_ZERO)) {return;} //subtract one cycle
    //NC
    if(Opcode == 0x30 && FLAG_ISSET(FLAG_CARRY)) {return;}
    //Z
    if(Opcode == 0x28 && !FLAG_ISSET(FLAG_ZERO)) {return;}
    //C
    if(Opcode == 0x38 && !FLAG_ISSET(FLAG_ZERO)) {return;}

    registers.PC--;
    registers.PC += offset; 
        
    
}

void JUMP16 (unsigned char Opcode, unsigned short Address) {

    //NZ
    if(Opcode == 0xC2 && FLAG_ISSET(FLAG_ZERO)) {return;} //subtract one cycle
    //NC
    if(Opcode == 0xD2 && FLAG_ISSET(FLAG_CARRY)) {return;}
    //Z
    if(Opcode == 0xCA && !FLAG_ISSET(FLAG_ZERO)) {return;}
    //C
    if(Opcode == 0xDA && !FLAG_ISSET(FLAG_ZERO)) {return;}

    registers.PC = Address; 
        
}

void CALL (unsigned char Opcode, unsigned short Address) {

    //NZ
    if(Opcode == 0xC4 && FLAG_ISSET(FLAG_ZERO)) {return;} //subtract one cycle
    //NC
    if(Opcode == 0xD4 && FLAG_ISSET(FLAG_CARRY)) {return;}
    //Z
    if(Opcode == 0xCC && !FLAG_ISSET(FLAG_ZERO)) {return;}
    //C
    if(Opcode == 0xDC && !FLAG_ISSET(FLAG_ZERO)) {return;}


    WriteStack(registers.PC);
    registers.PC = Address;

}

void RET (unsigned char Opcode){

    //NZ
    if(Opcode == 0xC0 && FLAG_ISSET(FLAG_ZERO)) {return;} //subtract one cycle
    //NC
    if(Opcode == 0xD0 && FLAG_ISSET(FLAG_CARRY)) {return;}
    //Z
    if(Opcode == 0xC8 && !FLAG_ISSET(FLAG_ZERO)) {return;}
    //C
    if(Opcode == 0xD8 && !FLAG_ISSET(FLAG_ZERO)) {return;}

    registers.PC = ReadStack();

}

void RETI (unsigned char Opcode) {
    interrupt.master = 1;
    RET(Opcode);
}

void RST (unsigned char Opcode) {

    struct opcode rst;
    rst.inst = Opcode;
    unsigned char RSTAddr[8] = {0x00, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38}; //Goes straight to these positions
    CALL(0x00, RSTAddr[rst.r1]);

}

void POP (unsigned char Opcode) {

    struct opcode pop;
    pop.inst = Opcode;
    *topRowRegs[pop.r1] = ReadStack();
    
}

void PUSH (unsigned char Opcode) {

    struct opcode push;
    push.inst = Opcode;
    WriteStack(*topRowRegs[push.r1]);
    
}

void CB (unsigned char Opcode, unsigned char operand) {

    struct opcode CB;
    CB.inst = operand;

    unsigned char direction;
    unsigned char *BitRegs[8] = {&registers.B, &registers.C, &registers.D, &registers.E, &registers.H, &registers.L, &memory[registers.HL], &registers.A};

    if(CB.low < 0x08) {direction = 1;} else {direction = 0;}

    unsigned char Bitnum;
    if (CB.high > 0x3 && CB.high < 0x8) {
        Bitnum = (CB.inst-0x4)/8;
    } else if (CB.high > 0x7 && CB.high < 0xC){
        Bitnum = (CB.inst-0x8)/8;
    } else if (CB.high > 0xB){
        Bitnum = (CB.inst-0xC)/8;
    }

    switch(CB.high){
                
        case 0x0: //RLC, RRC    
            ROTATE(BitRegs[CB.r2], 0, direction);
            break;

        case 0x1: //wrap THROUGH CY
            ROTATE(BitRegs[CB.r2], 1, direction);
            break;

        case 0x2: //SLA SRA
            SHIFT(*BitRegs[CB.r2], 0, direction);
            break;

        case 0x3: //SWAP, SRL

            if(CB.low < 0x08){ //SWAP
                struct opcode RegSwap;
                RegSwap.inst = *BitRegs[CB.r2];
                *BitRegs[CB.r2] = ((RegSwap.low<<4) + (RegSwap.high));
                return;

            } else { //SHIFT

                *BitRegs[CB.r2] = SHIFT(*BitRegs[CB.r2], 1, 0);
                return;

            }

        case 0x4:
        case 0x5:
        case 0x6:
        case 0x7: //BIT
           
            BIT(Bitnum, *BitRegs[CB.r2], 0);
            break;
        
        case 0x8:
        case 0x9:
        case 0xA:
        case 0xB: //RES
            
            *BitRegs[CB.r2] = BIT(Bitnum, *BitRegs[CB.r2], 2);
            break;

        case 0xC:
        case 0xD:
        case 0xE:
        case 0xF: //SET
            *BitRegs[CB.r2] = BIT(Bitnum, *BitRegs[CB.r2], 1);
            break;

            
    }
}

unsigned char SHIFT (unsigned char X, unsigned char logical, unsigned char direct) {
    //direct[0: right, 1: left]
    // logic/arithmetic[0: arithmetic, 1:logical]

    unsigned char C;

    if(direct){ //left
        C = ((X & 0x80) && (X & 0x80));

        if(!C) {
            FLAG_CLEAR(FLAG_CARRY);
        } else {
            FLAG_SET(FLAG_CARRY);
        }
        X <<= 1;

    } else { //right

        C = ((X & 0x01) && (X & 0x01));

        if(!C) {
            FLAG_CLEAR(FLAG_CARRY);
        } else {
            FLAG_SET(FLAG_CARRY);
        }

        if(!logical) {C = (X & 0x08);} else {C = 0;} //changes from 0 if arthmetic
        X = ((X>>1) + C);
        
    }

    FLAG_SET((X && X) * 0x80);
    FLAG_CLEAR(FLAG_N); //LOOK
    FLAG_CLEAR(FLAG_HALF);
    return X;

}

unsigned char BIT (unsigned char bit, unsigned char byte, unsigned char OP){
    //OP(operation): 0 = bit, 1 = set, 2 = res
    //No need for clocks here
    // xx b r [r(HL) = 110]

    //sending pointer
    switch(OP){
        case 0:
            unsigned char test = (byte & (0x01 << bit)); //Moves over until it reaches the num and ANDs it

            FLAG_SET((test && test) * 0x80);
            FLAG_CLEAR(FLAG_N); //LOOK
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