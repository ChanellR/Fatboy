#include <stdio.h>

#include "header.h"
#include "memory.h"
#include "control.h"

//Just focus on using Boot Rom for now 

unsigned char memory[0xFFFF];


unsigned char ReadByte (unsigned short Address) {

    switch(Address){
        case 0xFF04:
            return timer.DIV;
        case 0xFF05:
            return timer.TIMA;
        case 0x0FF06:
            return timer.TMA;
        case 0xFF07: //tima stacking
            return timer.TAC;
        case 0xFF00:
            return joypad.keys;
        default:
            

    }

   return memory[Address];
}

void WriteByte (unsigned short Address, unsigned char value) {

    if (Address < 0x8000) {
        printf("ROM SPACE, no write.");
        return;

    } else if (( Address >= 0xFEA0 ) && (Address < 0xFEFF) ) {
        printf("Restricted: no write");
        return;
    }  else if ( ( Address >= 0xE000 ) && (Address < 0xFE00) )
    {
        memory[Address] = value ;
        WriteByte(Address-0x2000, value);
   }


    switch(Address){
        case 0xFF04:
            timer.DIV = 0;
        case 0xFF05:
            timer.TIMA = value;
        case 0x0FF06:
            timer.TMA = value;
        case 0xFF07: //tima stacking
            timer.TAC = value;
        case 0xFF00:
            joypad.keys = value;
        default:
            memory[Address] = value;
    } 

    
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

