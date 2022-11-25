#pragma once
//taken from CINOOP, good organization

#define FLAG_ZERO (0x08)
#define FLAG_SUB (0x40)
#define FLAG_HALF (0x20)
#define FLAG_CARRY (0x10)

#define FLAG_ISZERO (registers.F & 0x80)
#define FLAG_ISSUB (registers.F & 0x40)
#define FLAG_ISHALF (registers.F & 0x20)
#define FLAG_ISCARRY (registers.F & 0x10) //if & and successful, produces non-zero value -> True

#define FLAG_ISSET(x) (registers.F & (x))
#define FLAG_SET(x) (registers.F |= (x))
#define FLAG_CLEAR(x) (registers.F &= ~(x))

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

struct interrupt {
    unsigned char master;
    unsigned char enable; //0xFFFF
    unsigned char flag; //0xFF0F
}extern interrupt;


struct timer {
    unsigned char DIV; // 0xFF04 always incrementing, rest on write
    unsigned char TIMA; // 0xFF05 resets to TMA on oveflow
    unsigned char TMA; // 0xFF06 timer modulo
    unsigned char TAC; // 0xFF07 timer control
}extern timer;

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
    signed char cycles;
    

}extern const instructions[256];

//Functions

void nop(void);
void loadRegB(unsigned char Opcode); //for registers and (HL)
void loadRegS(unsigned char Opcode); //load reg short
void load8bit(unsigned char Opcode, unsigned char operand); //load byte to Register
void load16bit(unsigned char Opcode, unsigned short operand);
void store8bit(unsigned short Address, unsigned char value);
void writeStack(unsigned short value);
void INCRegS(unsigned char Opcode);
void INCRegB(unsigned char Opcode);
void ADDC (unsigned char *destination, unsigned char val, unsigned char carry); //for use in other things

