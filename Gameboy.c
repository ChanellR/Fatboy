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
BYTE ROTATE (struct CPU *cpu, BYTE X, BYTE carry, char direct){
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
        

    }instruction;

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
            //Prefix CB
            return 3;
        case 0xF3:
            //DI
            return 4;
        case 0xFB:
            //EI
            return 5;   

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
