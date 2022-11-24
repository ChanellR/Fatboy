#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#define MAX_MEM 0xFFFF


const char Rom[] = "dmg_boot.bin";
int clock = 0; //Number of Machine Cycles (4 Clock cycles, )


typedef uint8_t BYTE;
typedef uint16_t Word;

struct CPU {
   
    Word SP, PC;
    
    // Registers 
    //is ordered Low, High in bits

    union {
        
        Word AF;
        struct {

            union{
                BYTE F;
                struct{
                    //Flag Register
                    BYTE : 4;
                    BYTE CY : 1;
                    BYTE h : 1;
                    BYTE n : 1;
                    BYTE Z : 1;
                };
            };

            BYTE A;
            
        };
        
    }; 
    union {
        
        Word BC;
        struct {
            BYTE C;
            BYTE B;
            
        };
        
    }; 
    union {
        
        Word DE;
        struct {
            BYTE E;
            BYTE D;
        };
        
    }; 
    union {
        
        Word HL;
        struct {
            BYTE L;
            BYTE H;
        };
        
    };

};

struct Mem {

    BYTE Data[MAX_MEM];
    
};

struct Instruction {

        union {

            BYTE inst;
            struct {
                BYTE low : 4;
                BYTE high : 4;
            };
            struct {
                BYTE r2 : 3;
                BYTE r1 : 3;
                BYTE : 2;
            };

            
        };
        

    };

//Catrige Header Mem Addrs
Word entryPoint[2] = {0x0100, 0x0103};
Word nintendoLogo[2] = {0x0104, 0x0133};
Word title[2] = {0x0134, 0x0143};
Word manuCode[2] = {0x013F, 0x0142}; //Newer chips
Word cgbFlag = 0x0143; //Newer Chips
Word licensee[2] = {0x0144, 0x0145};
Word sgbFlag = 0x0146;
Word cartType = 0x0147;
Word romSize = 0x0148;
Word ramSize = 0x0149;
Word destCode = 0x014A;
Word OldLicen = 0x014B;
Word masVer = 0x014C;
Word Hchecksum = 0x014D;
Word Gchecksum[2] = {0x014E, 0x014F};


//Memory Map
// 0x0000-0x3FFF: Permanently-mapped ROM bank.
// 0x4000-0x7FFF: Area for switchable ROM banks.
// 0x8000-0x9FFF: Video RAM.
// 0xA000-0xBFFF: Area for switchable external RAM banks.
// 0xC000-0xCFFF: Game Boy’s working RAM bank 0 .
// 0xD000-0xDFFF: Game Boy’s working RAM bank 1.
// 0xFE00-0xFE9F: Sprite Attribute Table.
// 0xFEA0-0xFEFF: Not Usable
// 0xFF00-0xFF7F: Devices’ Mappings. Used to access I/O devices.
// 0xFF80-0xFFFE: High RAM Area.
// 0xFFFF: Interrupt Enable Register


void Init (struct Mem *Mem){

    for (uint16_t i = 0; i < MAX_MEM; i++){
        Mem->Data[i] = 0x0000;
    }
}

void Reset (struct CPU *cpu) {    
    cpu->PC = cpu-> SP = cpu-> AF = cpu-> BC = cpu-> DE = cpu-> HL = 0;
}


void LoadRom (const char *Filename, struct Mem *Mem){

    FILE *f;
    f = fopen(Filename, "r");
    //Reads all x8000 bytes of Rom data
    fread(Mem, 0x8000, 1, f);

    printf("Completed Loading Rom \n");
    fclose(f);
}


void DisplayReg (struct CPU *ptr){

    printf("-----------------------\n");
    printf("A: 0x%X Z: %X n: %X h: %X CY: %X\nBC: 0x%X\nDE: 0x%X\nHL: 0x%X\nSP: 0x%X\nPC: 0x%X\nCycles: %d\n", 
    ptr->A, ptr->Z, ptr->n, ptr->h, ptr->CY, ptr->BC, ptr->DE, ptr->HL, ptr->SP, ptr->PC, clock);
    
}

//Start of Machine Cycle Code
//Everytime there is a read, there is a clock cycle
BYTE ReadPC (struct CPU *cpu, struct Mem *mem){
    
    BYTE output = mem->Data[cpu->PC];
    cpu->PC++;
    clock++;
    return output;

}

//Reads from specific Addr
BYTE ReadMem (struct CPU *cpu, struct Mem *mem, Word Addr){
    
    BYTE output = mem->Data[Addr];
    //iterates clock?
    clock++;
    return output;

}

Word PStack (struct CPU *cpu, struct Mem *mem, Word Addr, BYTE Push){
    //Push 4 Cycles, Pop 3 Cycles

    Word output;

    if(Push){

        cpu->SP--;
        mem->Data[cpu->SP] = ((Addr & 0xFF00)>>8);
        cpu->SP--;
        mem->Data[cpu->SP] = (Addr & 0x00FF);
        clock+=3;

    } else {

        output = mem->Data[cpu->SP];
        cpu->SP++;
        output += (mem->Data[cpu->SP]<<8);
        cpu->SP++;
        clock+=2;
    }

    return output;
}

Word Add16bit (struct CPU *cpu, Word A, Word B, BYTE WriteZ) {
    //don't edit Z for add HL, rr

    int sum = A + B;
   
    if (WriteZ) {cpu->Z = (sum == 0);}
    cpu->n = 0;

    A &= 0xFF; B &= 0xFF;
    cpu->h = ( (A + B) > 0xFF );

    cpu->CY = (sum > 0xFFFF);

    return sum;

}

Word Sub16bit (struct CPU *cpu, Word A, Word B, BYTE WriteZ){
    //don't edit Z for add HL, rr
    
    int diff = A - B;
    
    if (WriteZ) {cpu->Z = (diff == 0);}

    cpu->n = 0;

    A &= 0xFF, B &= 0xFF;
    cpu->h = ((A - B) && 0x100);

    cpu->CY = (diff < 0x0);

    return diff;
}

BYTE Add8bit (struct CPU *cpu, BYTE A, BYTE B, BYTE carry){
    

    int sum = A + B;
    
    cpu->Z = (sum == 0);
    cpu->n = 0;

    A &= 0x0F; B &= 0x0F;
    cpu->h = ((A + B) > 0x0F);

    cpu->CY = (sum > 0xFF);
    sum += (carry & cpu->CY); //Carry function

    return sum;

}

BYTE Sub8bit (struct CPU *cpu, BYTE A, BYTE B, BYTE carry){

    int diff = A - B;
    
    cpu->Z = (diff == 0);
    cpu->n = 1;

    A &= 0xF, B &= 0xF;

    cpu->h = ((A - B) & 0x10);

    cpu->CY = (diff < 0x0);
    diff -= (carry & cpu->CY);

    return diff;
}

//8 bit Logic
//NEED DAA & CPL
void AND (struct CPU *cpu, struct Mem *mem, BYTE X){

    cpu->A &= X;

    cpu->Z = (cpu->A == 0);
    cpu->n = 0;
    cpu->h = 1;
    cpu->CY = 0;
}

void XOR (struct CPU *cpu, struct Mem *mem, BYTE X){

    cpu->A ^= X;

    cpu->Z = (cpu->A == 0);
    cpu->n = 0;
    cpu->h = 0;
    cpu->CY = 0;
}

void OR (struct CPU *cpu, struct Mem *mem, BYTE X){

    cpu->A |= X;

    cpu->Z = (cpu->A == 0);
    cpu->n = 0;
    cpu->h = 0;
    cpu->CY = 0;
}

void CP (struct CPU *cpu, struct Mem *mem, BYTE X){

    cpu->A == X;

    cpu->Z = (cpu->A == 0);
    cpu->n = 0;
    cpu->h = 0;
    cpu->CY = 0;
}

void JUMP (struct CPU *cpu, Word Addr){

    cpu->PC = Addr;

}

void CallR (struct CPU *cpu, struct Mem *mem, Word Addr, BYTE call){
    //Make sure all clocks function properly
    //call [1: call, 2: reti, 0:...]

        if (call == 1){ //if it's a call 

            cpu->SP--;
            mem->Data[cpu->SP] = ((cpu->PC & 0xFF00)>>8);
            cpu->SP--;
            mem->Data[cpu->SP] = (cpu->PC & 0x00FF);
            cpu->PC = Addr;


        } else {
            
            //Actually seperates the parts, because otherwise they would overload and be lost
            cpu->PC = mem->Data[cpu->SP];
            cpu->SP++;
            cpu->PC += ((mem->Data[cpu->SP])<<8);
            cpu->SP++;

        }

}

BYTE BIT (struct CPU *cpu, struct Mem *mem, BYTE bit, BYTE byte, BYTE OP){
    //OP(operation): 0 = bit, 1 = set, 2 = res
    //No need for clocks here
    // xx b r [r(HL) = 110]

    //sending pointer
    switch(OP){
        case 0:
            BYTE test = (byte & (0x01 << bit)); //Moves over until it reaches the num and ANDs it
            cpu->Z = (test == 0);
            cpu->n = 0;
            cpu->h = 1;
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

//Rotate and Shift Instructions
BYTE ROTATE (struct CPU *cpu, BYTE X, BYTE carry, BYTE direct){
    //direct[0: right, 1: left]
    //carry[0: none, 1:through carry]

    if(direct){
        BYTE C = (X & 0x80 || X & 0x80);
        if(carry) {BYTE temp = C; C = cpu->CY; cpu->CY = temp;} else {cpu->CY = C;} //going through if 1
        X = (X<<1) + (C);
    } else {
        BYTE C = (X & 0x01 || X & 0x01);
        if(carry) {BYTE temp = C; C = cpu->CY; cpu->CY = temp;} else {cpu->CY = C;} //going through if 1
        X = (X>>1) + (C<<7);
    }

    cpu->Z = (X == 0);
    cpu->n = 0;
    cpu->h = 0;

    return X;
    
}

BYTE SHIFT (struct CPU *cpu, BYTE X, BYTE logical, char direct) {
    //direct[0: right, 1: left]
    // logic/arithmetic[0: arithmetic, 1:logical]

    if(direct){ //left
        BYTE C = ((X & 0x8) && (X & 0x80));
        cpu->CY = C;
        X <<= 1;

    } else { //right

        BYTE C = ((X & 0x01) && (X & 0x01));
        cpu->CY = C;

        if(!logical) {C = (X & 0x08);} else {C = 0;} //changes from 0 if arthmetic
        X = ((X>>1) + C);
        
    }

    cpu->Z = (X == 0);
    cpu->n = 0;
    cpu->h = 0;
    
    return X;

}

int ExecInstruction (struct CPU *cpu, struct Mem *mem) {

    struct CPU *C = cpu;
    struct Mem *M = mem;

    
    //All the 8 bit registers to be used in encoding
    BYTE *BitRegs[8] = {&cpu->B, &cpu->C, &cpu->D, &cpu->E, &cpu->H, &cpu->L, NULL, &cpu->A};
    //r1 can be used to distinguish these with their register pair code: dd
    
    //Register Pair dd, or qq [AF]
    //BC: 000, DE: 010, HL: 100, SP: 110 ?AF: 110[qq] -> 111
    Word *ByteRegs[8] = {&cpu->BC, NULL, &cpu->DE, NULL, &cpu->HL, NULL, &cpu->SP, &cpu->AF};

    //cc r1::[True: 011, NZ: 100, Z: 101, NC: 110, C:111]
    //              3         4        5       6      7
    

    struct Instruction instruction;

    instruction.inst = ReadPC (C, M);
    printf("Byte is 0X%X, the low: 0x%X, the high: 0x%X, the r2: %X, the r1: %X \n", instruction.inst, instruction.low, instruction.high, instruction.r2, instruction.r1);
    
    //First, large middle rows

    //A: 7, B: 0, C: 1, D: 2, E: 3, H: 4, L: 5, (HL): 6     [r]

    //JUMP Hardcode
    //cc r1::[True: 011, NZ: 100, Z: 101, NC: 110, C:111]
    //check for condition here

    //Single Byte offset JMP
    switch(instruction.inst){
        case 0x20: //NZ
            if(cpu->Z == 1) {cpu->PC++; clock++; return 0;} //move 1 forward to skip the offset
        case 0x30: //NC
            if(cpu->CY == 1) {cpu->PC++; clock++; return 0;}
        case 0x28: //Z
            if(cpu->Z != 1) {cpu->PC++; clock++; return 0;}
        case 0x38: //C
            if(cpu->CY != 1) {cpu->PC++; clock++; return 0;}
        case 0x18:
            signed char offset = ReadPC(C, M);
            clock++;
            JUMP(C, cpu->PC + offset);
            return 0;

    }

    //Word Address JMP
    switch(instruction.inst){
        case 0xC2: //NZ
            if(cpu->Z == 1) {cpu->PC+=2; clock++; return 0;} //move 1 forward to skip the offset
        case 0xD2: //NC
            if(cpu->CY == 1) {cpu->PC+=2; clock++; return 0;}
        case 0xCA: //Z
            if(cpu->Z != 1) {cpu->PC+=2; clock++; return 0;}
        case 0xDA: //C
            if(cpu->CY != 1) {cpu->PC+=2; clock++; return 0;}
        case 0xE9: //(HL)
            JUMP(C, cpu->HL); 
        case 0xC3:
            BYTE lowAddr = ReadPC(C, M);
            BYTE HighAddr = ReadPC(C, M);
            clock++;
            JUMP(C, (HighAddr<<4) + lowAddr);
            return 0;
        
        

    }

    //CAll and RE**
    switch(instruction.inst){
        case 0xC4: //NZ
            if(cpu->Z == 1) {cpu->PC+=2; clock++; return 0;} //move 1 forward to skip the offset
        case 0xD4: //NC
            if(cpu->CY == 1) {cpu->PC+=2; clock++; return 0;}
        case 0xCC: //Z
            if(cpu->Z != 1) {cpu->PC+=2; clock++; return 0;}
        case 0xDC: //C
            if(cpu->CY != 1) {cpu->PC+=2; clock++; return 0;}
        case 0xCD: //Always true

            BYTE lowAddr = ReadPC(C, M);
            BYTE HighAddr = ReadPC(C, M);
            clock+=3; // 24/12
            CallR(C, M, (HighAddr<<4) + lowAddr, 1);
            return 0;
        
        //RET
        case 0xC0: //NZ
            if(cpu->Z == 1) {clock++; return 0;} //move 1 forward to skip the offset
        case 0xD0: //NC
            if(cpu->CY == 1) {clock++; return 0;}
        case 0xC8: //Z
            if(cpu->Z != 1) {clock++; return 0;}
        case 0xD8: //C
            if(cpu->CY != 1) {clock++; return 0;}
        case 0xD9:
            //INTERRUPT FLAG IEM
        case 0xC9:
            CallR(C, M, 0, 0);
            clock+=3;
            return 0;

        //RST Cases
        case 0xC7:
        case 0xD7:
        case 0xE7:
        case 0xF7:
        case 0xCF:
        case 0xDF:
        case 0xEF:
        case 0xFF:
            BYTE RSTAddr[8] = {0x00, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38}; //Goes straight to these positions
            CallR (C, M, RSTAddr[instruction.r1], 1);
            clock+=3;
            return 0;

    }

    //PUSH & POP
    switch (instruction.low){
        case 0x01:
            if (instruction.high > 0xB) {
                //BC: 000, DE: 010, HL: 100, SP: 110 ?AF: 110[qq] -> 111
                if (instruction.r1 == 0x110) {cpu->AF = PStack(C, M, 0, 0); return 0;} //PSTACK handles clocks
                *ByteRegs[instruction.r1] = PStack(C, M, 0, 0);
            }
        case 0x05:
            if (instruction.high > 0xB) {
                //BC: 000, DE: 010, HL: 100, SP: 110 ?AF: 110[qq] -> 111
                if (instruction.r1 == 0x110) {PStack(C, M, cpu->AF, 1); return 0;} //PSTACK handles clocks
                PStack(C, M, *ByteRegs[instruction.r1], 1);
            }
    }

    //BC: 000, DE: 010, HL: 100, SP: 110 r1
        // B: 000, D: 010, H:100, (HL): 110 r1
        // C: 001, E: 011, L: 101, A:111 
        Word *topRowRegs[7] = {&cpu->BC, NULL, &cpu->DE, NULL, &cpu->HL, NULL, &cpu->SP};
        BYTE *topRowRegsHigh[6] = {&cpu->B, NULL, &cpu->D, NULL, &cpu->H, NULL}; //(HL) should be at end
        BYTE *topRowRegsHigh2[8] = {NULL, &cpu->C, NULL, &cpu->E, NULL, &cpu->L, NULL, &cpu->A};
    //16bit+ LD

    switch (instruction.inst){
        //r2 [100: (a8),  (a16) and also 100:(C) 110: A, ]
        case 0xE0: //LD (FF00 + )
            BYTE n_offset = ReadPC(C, M);
            mem->Data[0xFF00 + n_offset] = cpu->A;
            clock++;
            return 0;
        case 0xF0:
            n_offset = ReadPC(C, M);
            cpu->A =  mem->Data[0xFF00 + n_offset];
            clock++;
            return 0;
        
        case 0xE2: //LD (C) A
            mem->Data[0xFF00 + cpu->C] = cpu->A; 
            clock++;
            return 0;
            
        case 0xF2:
            cpu->A = mem->Data[0xFF00 + cpu->C];
            clock++;
            return 0;

        case 0xEA: //LD (a16) , A
            Word a16_addr = ReadPC(C, M) + (ReadPC(C, M) << 8);
            mem->Data[a16_addr] = cpu->A;
            clock++;
            return 0;
        case 0xFA:
            a16_addr = ReadPC(C, M) + (ReadPC(C, M) << 8);
            cpu->A = mem->Data[a16_addr];
            clock++;
            return 0; 

        case 0xF8: //LD HL, SP+r8
            n_offset = ReadPC(C, M);
            cpu->HL = cpu->SP + n_offset;
            clock++;
            return 0;

        case 0xF9:
            cpu->SP = cpu->HL;
            clock++;
            return 0;
        
        case 0x08: //LD (a16), SP
            a16_addr = ReadPC(C, M) + (ReadPC(C, M) << 8);
            mem->Data[a16_addr] = cpu->SP;
            clock += 2;
            return 0;

        case 0x01:
        case 0x11:
        case 0x21:
        case 0x31:

            Word d16 = ReadPC(C, M) + (ReadPC(C, M) << 8);
            *topRowRegs[instruction.r1] = d16;
            return 0;

        case 0x02:
        case 0x12:

            mem->Data[*topRowRegs[instruction.r1]] = cpu->A;    
            clock++;
            return 0;

        case 0x22:
            
            mem->Data[cpu->HL] = cpu->A;
            cpu->HL++;
            clock++;
            return 0;

        case 0x32: //LD (HL-), A

            mem->Data[cpu->HL] = cpu->A;
            cpu->HL--;
            clock++;
            return 0;
        
        case 0x06:
        case 0x16:
        case 0x26:
        case 0x36:

            BYTE d8 = ReadPC(C, M);
            if(instruction.r1 == 0x110) {mem->Data[cpu->HL] = d8; clock++; return 0;}
            *topRowRegsHigh[instruction.r1] = d8;
            return 0;
        
        case 0x0A: //LD A, (BC)

            cpu->A = mem->Data[cpu->BC];   
            clock++;
            return 0;

        case 0x1A:

            cpu->A = mem->Data[cpu->DE];   
            clock++;
            return 0;

        case 0x2A:
            
            cpu->A = mem->Data[cpu->HL];
            cpu->HL++;
            clock++;
            return 0;

        case 0x3A: //LD A, (HL-)

            cpu->A = mem->Data[cpu->HL];
            cpu->HL--;
            clock++;
            return 0;
        
        case 0x0E:
        case 0x1E:
        case 0x2E:
        case 0x3E:
            d8 = ReadPC(C, M);
            *topRowRegsHigh2[instruction.r1] = d8;
            return 0;

    }

    //Arithmetic/Logic instructions top ROW
    switch(instruction.inst){

        case 0x03:
        case 0x13:
        case 0x23:
        case 0x33: //INC 16 bit registers

        *topRowRegs[instruction.r1]++;
        clock++;
        return 0;

        case 0x04:
        case 0x14:
        case 0x24:
        case 0x34: //INC B, and (HL)

            if (instruction.r1 == 0x110) {mem->Data[cpu->HL] = Add8bit(C, mem->Data[cpu->HL], 1, 0); clock+= 2; return 0;}
            *topRowRegsHigh[instruction.r1] = Add8bit(C, *topRowRegsHigh[instruction.r1], 1, 0);
            return 0;

        case 0x05:
        case 0x15:
        case 0x25:
        case 0x35: //DEC B, and (HL)

            if (instruction.r1 == 0x110) {mem->Data[cpu->HL] = Sub8bit(C, mem->Data[cpu->HL], 1, 0); clock+= 2; return 0;}
            *topRowRegsHigh[instruction.r1] = Sub8bit(C, *topRowRegsHigh[instruction.r1], 1, 0);
            return 0;
        
        //d8 arithmetic 
        case 0xC6:

            BYTE d8 = ReadPC(C, M);
            cpu->A = Add8bit(C, cpu->A, d8, 0);
            return 0;

        case 0xD6:

            d8 = ReadPC(C, M);
            cpu->A = Sub8bit(C, cpu->A, d8, 0);
            return 0;

        case 0xE6:

            d8 = ReadPC(C, M);
            AND(C, M, d8);
            return 0;

        case 0xF6:

            d8 = ReadPC(C, M);
            OR(C, M, d8);
            return 0;
        

         //More arithmetic ADC, SBC, XOR, CP
        case 0xCE:

            d8 = ReadPC(C, M);
            cpu->A = Add8bit(C, cpu->A, d8, 1);
            return 0;

        case 0xDE:

            d8 = ReadPC(C, M);
            cpu->A = Sub8bit(C, cpu->A, d8, 1);
            return 0;

        case 0xEE:

            d8 = ReadPC(C, M);
            XOR(C, M, d8);
            return 0;

        case 0xFE:

            d8 = ReadPC(C, M);
            CP(C, M, d8);
            return 0;

        //16 bit Arithmetic
        case 0xE8: //ADD SP, r8

            signed short r8 = ReadPC(C, M);
            if (r8>0x00) {
                cpu->SP = Add16bit(C, cpu->SP, r8, 0);
                cpu->Z = 0;
                clock += 2;
                return 0;
            }
            //if negative
            r8 = ~(r8) + 1;
            cpu->SP = Sub16bit(C, cpu->SP, r8, 0);
            cpu->Z = 0;
            clock += 2;
            return 0;
        
        case 0x09:
        case 0x19:
        case 0x29:
        case 0x39: //AD 16-bit registers

            cpu->HL = Add16bit(C, cpu->HL, *topRowRegs[instruction.r1-1], 0);
            clock++;
            return 0;

        case 0x0B:
        case 0x1B:
        case 0x2B:
        case 0x3B: //DEC 16 bit 
            
            *topRowRegs[instruction.r1-1]--;
            clock++;
            return 0;

        case 0x0C:
        case 0x1C:
        case 0x2C:
        case 0x3C: //INC 8 bit regs

            *topRowRegsHigh2[instruction.r1] = Add8bit(C, *topRowRegsHigh2[instruction.r1], 1, 0);
            return 0;

        case 0x0D:
        case 0x1D:
        case 0x2D:
        case 0x3D: //DEC 8 bit regs

            *topRowRegsHigh2[instruction.r1] = Sub8bit(C, *topRowRegsHigh2[instruction.r1], 1, 0);
            return 0;

        case 0x7: //ROTATIONS
            cpu->A = ROTATE(C, cpu->A, 0, 1);
            return 0;

        case 0x17:
            cpu->A = ROTATE(C, cpu->A, 1, 1);
            return 0;

        case 0x27:
            cpu->A = ROTATE(C, cpu->A, 0, 0);
            return 0;

        case 0x37:
            cpu->A = ROTATE(C, cpu->A, 1, 0);
            return 0;

    }

    //Control things
    switch (instruction.inst){
        case 0x00:
            //NOP
            return 0;
        case 0x010:
            ReadPC(C, M); //2 Machine Cycles
            //STOP
            return 1; 
        case 0x76:
            //Halt
            return 2;
        case 0xCB:

            struct Instruction CB;
            CB.inst = ReadPC(C, M);
            BYTE direction;
            if(CB.low < 0x08) {direction = 1;} else {direction = 0;}

            BYTE Bitnum;
            if (CB.high > 0x3 && CB.high < 0x8) {
                Bitnum = (CB.inst-0x4)/8;
                
            } else if (CB.high > 0x7 && CB.high < 0xC){
                Bitnum = (CB.inst-0x8)/8;
            } else if (CB.high > 0xB){
                Bitnum = (CB.inst-0xC)/8;
            }
            // CB PREFIX SWITCH STATEMENT

            switch(CB.high){
                
                case 0x0: //RLC, RRC
                    if(CB.r2 == 0x06) { mem->Data[cpu->HL] = ROTATE(C, mem->Data[cpu->HL], 0, direction); clock += 2; return 0;}    
                    *BitRegs[CB.r2] = ROTATE(C, *BitRegs[CB.r2], 0, direction);
                    return 0;

                case 0x1: //wrap THROUGH CY
                    if(CB.r2 == 0x06) { mem->Data[cpu->HL] = ROTATE(C, mem->Data[cpu->HL], 1, direction); clock += 2; return 0;}    
                    *BitRegs[CB.r2] = ROTATE(C, *BitRegs[CB.r2], 1, direction);
                    return 0;

                case 0x2: //SLA SRA
                    if(CB.r2 == 0x06) { mem->Data[cpu->HL] = SHIFT(C, mem->Data[cpu->HL], 0, direction); clock += 2; return 0;}
                    *BitRegs[CB.r2] = SHIFT(C, *BitRegs[CB.r2], 0, direction);
                    return 0;

                case 0x3: //SWAP, SRL

                    if(CB.low < 0x08){ //SWAP
                        if (CB.r2 == 0x06) {
                            struct Instruction memHL;
                            memHL.inst = mem->Data[cpu->HL];
                            mem->Data[cpu->HL] = (memHL.low<<4) + (memHL.high); //SWAP
                            return 0;
                        }

                        struct Instruction RegSwap;
                        RegSwap.inst = *BitRegs[CB.r2];
                        *BitRegs[CB.r2] = ((RegSwap.low<<4) + (RegSwap.high));
                        return 0;

                    }

                    if (CB.r2 == 0x06) {mem->Data[cpu->HL] = SHIFT(C, mem->Data[cpu->HL], 1, 0); clock += 2; return 0;}
                    *BitRegs[CB.r2] = SHIFT(C, *BitRegs[CB.r2], 1, 0);
                    return 0;
                
                case 0x4:
                case 0x5:
                case 0x6:
                case 0x7: //BIT
                    if (CB.r2 == 0x06) {BIT(C, M, Bitnum, mem->Data[cpu->HL], 0); clock += 2; return 0;}
                    BIT(C, M, Bitnum, *BitRegs[CB.r2], 0);\
                    return 0;
                
                case 0x8:
                case 0x9:
                case 0xA:
                case 0xB: //RES
                    if (CB.r2 == 0x06) {mem->Data[cpu->HL] = BIT(C, M, Bitnum, mem->Data[cpu->HL], 2); clock += 2; return 0;}
                    *BitRegs[CB.r2] = BIT(C, M, Bitnum, *BitRegs[CB.r2], 2);
                    return 0;

                case 0xC:
                case 0xD:
                case 0xE:
                case 0xF: //SET
                    if (CB.r2 == 0x06) {mem->Data[cpu->HL] = BIT(C, M, Bitnum, mem->Data[cpu->HL], 1); clock += 2; return 0;}
                    *BitRegs[CB.r2] = BIT(C, M, Bitnum, *BitRegs[CB.r2], 1);
                    return 0;

            }
            


            //Not hard to implement
            
            return 3;
        case 0xF3:
            //DI
            return 4;
        case 0xFB:
            //EI
            return 5;   

    }


    // THE LAST FEW DAA
    switch(instruction.inst) {
        case 0x27:
            //DAA
        case 0x37:
            //SCF
            cpu->CY = 1;
            cpu->n = 0;
            cpu->h = 0;
            return 0;
        case 0x2F:
            //CPL
            cpu->A ^= 0xFF;
            cpu->n = 1;
            cpu->h = 1;
            return 0;

        case 0x3F:
            //CCF
            cpu->CY ^= 1;
            cpu->n = 0;
            cpu->h = 0;
            return 0;
    }

    //INSTRUCTION SET
    switch (instruction.high) {
        
        case 0x7:
        case 0x4:
        case 0x5:
        case 0x6:
        
            // Almost all LD

            if (instruction.r2 == 0x06) 
            { //Catches (HL)
                *BitRegs[instruction.r1] = ReadMem(C, M, cpu->HL);
                break;
            } else if (instruction.r1 == 0x06)
            {

                M->Data[cpu->HL] = *BitRegs[instruction.r2];
                clock++;
                break;
            }

            *BitRegs[instruction.r1] = *BitRegs[instruction.r2];
            break;

        case 0x8:
            //8 bit Logic/Arithmetic (16bit on x6 and xD)
            //(ADD & ADC)
            if (instruction.r2 == 0x6){ //Low checks if BIT 3 is 1
                cpu->A = Add8bit(C, cpu->A, ReadMem(C, M, cpu->HL), instruction.low & 0x8);
                break;
            }

            cpu->A = Add8bit(C, cpu->A, *BitRegs[instruction.r2], instruction.low & 0x8);
            break;

        case 0x9:
            //SUB and SBC
             if (instruction.r2 == 0x6){
                cpu->A = Sub8bit(C, cpu->A, ReadMem(C, M, cpu->HL), instruction.low & 0x8);
                break;
            }

            cpu->A = Sub8bit(C, cpu->A, *BitRegs[instruction.r2], instruction.low & 0x8);
            break;

        case 0xA:
            //AND
            if(instruction.low < 0x8) {
                if(instruction.r2 == 0x06) {
                    AND(C, M, ReadMem(C, M, cpu->HL));
                    break;
                }

                AND(C, M, *BitRegs[instruction.r2]);
                break;
            }

            //XOR
            if(instruction.r2 == 0x06) {
                    XOR(C, M, ReadMem(C, M, cpu->HL));
                    break;
            }    

            XOR(C, M, *BitRegs[instruction.r2]);
            break;

        case 0xB:
            //OR
            if(instruction.low < 0x8) {
                if(instruction.r2 == 0x06) {
                    AND(C, M, ReadMem(C, M, cpu->HL));
                    break;
                }

                AND(C, M, *BitRegs[instruction.r2]);
                break;
            }

            //CP subtract with no assign
            if(instruction.r2 == 0x06) {
                    Sub8bit(C, cpu->A, ReadMem(C, M, cpu->HL), 0);
                    break;
            }    

            Sub8bit(C, cpu->A, *BitRegs[instruction.r2], 0);
            break;

    
        //Second, columns
        default:
                    
                    
    }
    
}

int main(void){

    struct CPU cpu; 
    struct Mem mem;
    char input;
    
    Init( &mem );
    Reset( &cpu );
    // LoadRom ( Rom , &mem);
    mem.Data[0] = 0x00A0;
    mem.Data[1] = 0x0088;
    cpu.SP = 0xFFFE;

    cpu.A = 0x03;
    cpu.B = 0x04;
    
    

    while (input != 'n'){
        ExecInstruction ( &cpu, &mem);
        DisplayReg (&cpu );
        
        //user deactivated loop
        printf("Continue? [y/n]: ");
        scanf(" %c", &input);
    }
    

}
    
   
    
//
