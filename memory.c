#include <stdio.h>

#include "header.h"
#include "memory.h"

//Just focus on using Boot Rom for now 

unsigned char memory[0xFFFF];

unsigned char ReadByte (unsigned short Address) {
    return memory[Address];
}

void WriteByte (unsigned short Address, unsigned char value) {
    memory[Address] = value;
}

unsigned short ReadShort (unsigned short Address) {
    return (unsigned short)( memory[Address] | (memory[Address + 1]<<8) );
}

void WriteShort (unsigned short Address, unsigned short value) {
    
    WriteByte(Address, (unsigned char)(value & 0xFF));
    WriteByte(Address + 1, (unsigned char)( (value & 0xFF00) >> 8) );
}

void WriteStack (unsigned short value) {

    registers.SP -= 2;
    WriteShort (registers.SP, value);
    
}

unsigned short ReadStack (void) {
    unsigned short output = ReadShort(registers.SP);
    registers.SP += 2;
    return output;
}

