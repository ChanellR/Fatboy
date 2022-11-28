#include <windows.h>
#include <stdio.h>

#include "header.h"
#include "memory.h"
#include "control.h"
#include "display.h"
#include "gpu.h"


unsigned char instruction;
int instSkips;
unsigned short operand = 0;
int currentlyskipping = 1;

int CreateBox(char *label);


int realtimeDebug(void) {
	
	int output;
    // unsigned short Breakpoint = 0x1F7D;
    
    // if (instSkips) {
    //     instSkips--;
    // }

    // if(registers.PC == Breakpoint) {
    //     currentlyskipping = 0;
    // }

    // if(currentlyskipping) {
    //     instSkips++;
    // }

   

    instruction = ReadByte(registers.PC++);
  

    switch (instructions[instruction].operand_length){
        
        case -4:
            operand = ReadByte(registers.PC++);
            // output = CreateBox("after operand (byte)");
            ((void (*)(unsigned char, signed char))instructions[instruction].function)(instruction, (signed char) operand);
            
            
            break;
        case -3:

            operand = ReadShort(registers.PC);
            // output = CreateBox("after operand (short)");
            registers.PC += 2;

            ((void (*)(unsigned char, unsigned short))instructions[instruction].function)(instruction, operand);
            
            break;

        case -2:
            operand = ReadByte(registers.PC++);
            //  output = CreateBox("after operand (byte)");
            ((void (*)(unsigned char, unsigned char))instructions[instruction].function)(instruction, operand);
            break;
        case -1:
            operand = 0;
            output = CreateBox("no operand");
            ((void (*)(unsigned char))instructions[instruction].function)(instruction);
            break;

        case 0:
        operand = 0;
        output = CreateBox("no op/inst passing");
            ((void (*)())instructions[instruction].function)();
             
            break;

        case 1:
            operand = ReadByte(registers.PC++);
            // output = CreateBox("after operand (byte)");
            ((void (*)(unsigned char))instructions[instruction].function)(operand);
             
             
            break;

    }
	
  
	// if(instructions[instruction].operand_length) debugMessageP += 
	// else debugMessageP += sprintf(debugMessageP, instruction);
    

    return instructions[instruction].cycles;
}


int CreateBox (char *label){
    
    char debugMessage[5000];
    char *debugMessageP = debugMessage;

    if (instSkips) {
        //skip if there are remaining skips
        return 0;
    }

    debugMessageP += sprintf(debugMessageP, "%s \ninstruction: 0x%02X Operand: 0x%04X \nOPID: %d, cycles: %u", label, instruction, operand, instructions[instruction].operand_length, instructions[instruction].cycles);
	debugMessageP += sprintf(debugMessageP, "\n\nAF: 0x%04X\n", registers.AF);
	debugMessageP += sprintf(debugMessageP, "BC: 0x%04X\n", registers.BC);
	debugMessageP += sprintf(debugMessageP, "DE: 0x%04X\n", registers.DE);
	debugMessageP += sprintf(debugMessageP, "HL: 0x%04X\n", registers.HL);
	debugMessageP += sprintf(debugMessageP, "SP: 0x%04X\n", registers.SP);
	debugMessageP += sprintf(debugMessageP, "PC : 0x%04X\n", registers.PC );
	debugMessageP += sprintf(debugMessageP, "\nIME: 0x%02X\n", interrupt.master);
	debugMessageP += sprintf(debugMessageP, "IE: 0x%02X\n", interrupt.enable);
	debugMessageP += sprintf(debugMessageP, "IF: 0x%02X\n", interrupt.flag);
    debugMessageP += sprintf(debugMessageP, "LY: %d, LYC: %d, DIV: %d \n", lcd.LY, lcd.LYC, timer.DIV);
	
	debugMessageP += sprintf(debugMessageP, "\nTry again: LogMemory, continue: +1\n");
	
    int response = MessageBox(NULL, debugMessage, "Debug Step", MB_DEFBUTTON3 | MB_CANCELTRYCONTINUE | MB_ICONINFORMATION);
    
    if (response == 10) { //try again 10 cancel: 2 continue: 11
        //Log Memory
        
        LogMemory();
        instSkips = 5;
    } 

    if (response == 2) {
        exit(0);
    }

    return response;
}
	

