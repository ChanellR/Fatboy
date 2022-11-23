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


int ExecInstruction (struct CPU *cpu, struct Mem *mem) {

    struct CPU *C = cpu;
    struct Mem *M = mem;

    
    //All the 8 bit registers to be used in encoding
    BYTE *BitRegs[8] = {&cpu->B, &cpu->C, &cpu->D, &cpu->E, &cpu->H, &cpu->L, NULL, &cpu->A};
    //r1 can be used to distinguish these with their register pair code: dd
    
    //Register Pair dd, or qq [AF]
    //BC: 000, DE: 010, HL: 100, SP: 110 ?AF: 110[qq] -> 111
    Word *ByteRegs[8] = {&cpu->BC, NULL, &cpu->DE, NULL, &cpu->HL, NULL, &cpu->SP, &cpu->AF};

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
        case 0xC8:
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
        switch (instruction.low){
            
            case 0x0:
                if (instruction.high < 0x4){
                    //JR
                    
                    
                }

        }

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
    cpu.A = 0x03;
    cpu.B = 0x04;
    
    

    while (input != 'n'){
        ExecInstruction ( &cpu, &mem);
        DisplayReg (&cpu );
        
        
        printf("Continue? [y/n]: ");
        scanf(" %c", &input);
    }
    

}
    
   
    

