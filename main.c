#include <stdio.h>

#include "header.h"
#include "memory.h"

struct timer timer;
struct interrupt interrupt;
struct registers registers;


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

const struct Instruction instructions[256] = {

    {0, WriteByte, 1},   //0x00 //-3: word LD, -2: byte LD, -1: register specific, 0: none, 1:byte, 2: word
    {-3, load16bit, 3},
    {-1, loadRegS, 2},
    {-1, INCRegS, 2},
    {-1, INCRegB, 1},   //0x04   
    // {-1, dec b, 1},   //0x05
    // {-2, load B, d8, 2},   //0x06
    // {0, rlca, 1},   //0x07
    // {-3, load a16 SP, 5},   //0x08
    // {-1, add HL BC, 2},   //0x09
    // {-1, load A BC, 2}, //0x0a
    // {-1, dec BC, 2},  //0x0b
    // {-1, inc C, 1},   //0x0c
    // {-1, dec C, 1},   //0x0d
    // {-2, load C d8, 2},   //0x0e
    // {0, rrca, 1},   //0x0f
    
};


int main (void) {
    //the asterisk is an empty function constructor, which calls the pointer function from instructions i suppose

    unsigned char instruction = ReadByte(registers.PC);

    switch (instructions[instruction].operand_length){

        case -3:
           

    }

    registers.HL = 0;
    registers.B = 4;
    ((void (*)(unsigned char))instructions[2].function)(0x70);
    printf("%X", memory[registers.HL]);

}

//BC: 000, DE: 010, HL: 100, SP: 110 r1
// B: 000, D: 010, H:100, (HL): 110 r1
// C: 001, E: 011, L: 101, A:111 
unsigned short *topRowRegs[7] = {&registers.BC, NULL, &registers.DE, NULL, &registers.HL, NULL, &registers.SP};
unsigned char *topRowRegsHigh[6] = {&registers.B, NULL, &registers.D, NULL, &registers.H, NULL}; //(HL) should be at end
unsigned char *topRowRegsHigh2[8] = {NULL, &registers.C, NULL, &registers.E, NULL, &registers.L, NULL, &registers.A};


void nop(void) {}

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

//Not on main 
void ADDC (unsigned char *destination, unsigned char val, unsigned char carry){

    unsigned char sum = *destination + val;
    if (sum == 0) {FLAG_SET(FLAG_ZERO);} //Z
    FLAG_CLEAR(FLAG_SUB);

    unsigned char A = *destination & 0x0F; 
    unsigned char B = val & 0x0F;

    if ((A + B) > 0x0F) {FLAG_SET(FLAG_HALF);}
    if( sum > 0xFF ) {FLAG_SET(FLAG_CARRY);}

    sum += (FLAG_ISCARRY & carry);

    *destination = sum;

}


