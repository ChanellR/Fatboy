#pragma once
#include <stdbool.h>
//taken from CINOOP, good organization

#define FLAG_ZERO (0x80)
#define FLAG_N (0x40)
#define FLAG_HALF (0x20)
#define FLAG_CARRY (0x10)

#define FLAG_ISZERO (registers.F & 0x80)
#define FLAG_ISN (registers.F & 0x40)
#define FLAG_ISHALF (registers.F & 0x20)
#define FLAG_ISCARRY (registers.F & 0x10) //if & and successful, produces non-zero value -> True

#define FLAG_ISSET(x) (registers.F & (x))
#define FLAG_SET(x) (registers.F |= (x))
#define FLAG_CLEAR(x) (registers.F &= ~x)


extern int timercounter;
extern int dividercounter;
extern int ScanlineCounter;
extern char CurrentRom[40];
extern int RomLoaded;

struct registers {

    unsigned short SP, PC;  

    // Registers 
    //is ordered Low, High in bits

    union {

        unsigned short AF;
        struct {
            unsigned char F;
            unsigned char A;        
        };
        
    }; 
    union {
        unsigned short BC;
        struct {
            unsigned char C;
            unsigned char B;
            
        };
        
    }; 
    union {
        
        unsigned short DE;
        struct {
            unsigned char E;
            unsigned char D;
        };
        
    }; 
    union {
        
        unsigned short HL;
        struct {
            unsigned char L;
            unsigned char H;
        };
        
    };

}extern registers;

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

extern unsigned char memory[0xFFFF];


//struct renderer

//Have load instructions have -1 operand length, so that the instruction can be passed in and decoded. 
//for parsing LD, etc.
struct opcode {

        union {

            unsigned char inst;
            struct {
                unsigned int low : 4;
                unsigned int high : 4;
            };
            struct {
                unsigned int r2 : 3;
                // 0b00[r1][r2]
                unsigned int r1 : 3;
            };

        };
        
}extern opcode;

struct Instruction {
    signed char operand_length; //-3: word LD, -2: byte LD, -1: register specific, 0: none, 1:byte, 2: word
    void *function;
    int cycles;
    

}extern const instructions[256];

//Functions

void nop(void);
void HALT (void);
void STOP (void);

void loadRegB(unsigned char Opcode); //for registers and (HL)
void loadRegS(unsigned char Opcode); //load reg short
void load8bit(unsigned char Opcode, unsigned char operand); //load byte to Register

void load16bit(unsigned char Opcode, unsigned short operand);

void INCRegS(unsigned char Opcode);
void INCRegB(unsigned char Opcode);

void ADDC (unsigned char *destination, unsigned char val, unsigned char carry); //for use in other things
void DECRegB (unsigned char Opcode);
void DECRegS (unsigned char Opcode);
void SUBC (unsigned char *destination, unsigned char val, unsigned char carry);
void ADDHL(unsigned char Opcode);
void ADDCS (unsigned short *destination, unsigned short val, unsigned char carry);


void RR (unsigned char Opcode);
void ROTATE (unsigned char *object, unsigned char carry, unsigned char direct);

void JUMP8 (unsigned char Opcode, signed char offset);
void JUMP16 (unsigned char Opcode, unsigned short Address);
void CALL (unsigned char Opcode, unsigned short Address);
void JUMPHL (void);

void CPL (void);
void SCF (void);
void CCF (void);
void DAA (void);

void ADD (unsigned char Opcode);
void ADD8 (unsigned char Opcode, unsigned char operand);
void SUB (unsigned char Opcode);

void AND (unsigned char Opcode);
void XOR (unsigned char Opcode);
void OR (unsigned char Opcode);
void CP (unsigned char Opcode);

void RET (unsigned char Opcode);

void RETI (unsigned char Opcode);
void DI (void);
void EI (void);

void RST (unsigned char Opcode);
void POP (unsigned char Opcode);
void PUSH (unsigned char Opcode);

void CB (unsigned char Opcode, unsigned char operand);
unsigned char SHIFT (unsigned char X, unsigned char logical, unsigned char direct);
unsigned char BIT (unsigned char bit, unsigned char byte, unsigned char OP);

void SUB8 (unsigned char Opcode, unsigned char operand);
void AND8 (unsigned char Opcode, unsigned char operand);
void XOR8 (unsigned char Opcode, unsigned char operand);
void OR8 (unsigned char Opcode, unsigned char operand);
void CP8 (unsigned char Opcode, unsigned char operand);

void ADDSP (unsigned char Opcode, signed char operand);

void HandleInterrupt (void);

int CpuStep (void);

int realtimeDebug(void);
int GetFrequency(void);
int IsClockEnabled(void);
void UpdateDivider (int cycles);
void UpdateTiming (int cycles);
void RequestInterrupt(int val);
void LoadRom (char * Filename);
void Update (void);

void Reset (void);
