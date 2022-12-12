#include <windows.h>
#include <stdio.h>

#include "header.h"
#include "memory.h"
#include "control.h"
#include "display.h"
#include "gpu.h"


unsigned char instruction;
int instSkips = 1;
unsigned short operand = 0;
int currentlyskipping = 1;
unsigned short Breakpoint = 0x4BED;
int go = 0;

int CreateBox(char *label);


int realtimeDebug(void) {
	
	int output;

    
    instSkips = 0;
    CreateBox("");
    instSkips = 1;
    // int go = 1;
    // if(registers.PC == 0xC365 && (go == 1)){
    //     go = 0;
    //     printf("%04x", ReadByte(0xFF44));
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
            if (instruction == 0x18 && operand == 0xFE){
                exit(0); //infinite JMP -2 loop from blargs
            }

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

    //only after op
    
    
	if (instSkips) {
        instSkips--;
    }

    if(registers.PC == Breakpoint) {
        currentlyskipping = 0;
    }

    if(currentlyskipping) {
        instSkips++;
    }


	// if(instructions[instruction].operand_length) debugMessageP += 
	// else debugMessageP += sprintf(debugMessageP, instruction);
    

    return instructions[instruction].cycles;
}


int CreateBox (char *label){
    
    char debugMessage[7000];
    char *debugMessageP = debugMessage;

    
    if (instSkips) {
        //skip if there are remaining skips
        return 0;
    }

    
    debugMessageP += sprintf(debugMessageP, "A: %02X F: %02X B: %02X C: %02X D: %02X E: %02X H: %02X L: %02X SP: %04X PC: 00:%04X (%02X %02X %02X %02X)\n", 
                            registers.A, registers.F, registers.B, registers.C, registers.D, registers.E, registers.H, registers.L, registers.SP, registers.PC, 
                            ReadByte(registers.PC), ReadByte(registers.PC + 1), ReadByte(registers.PC + 2), ReadByte(registers.PC + 3));

    FILE *ptr;
    
    ptr = fopen("Log.txt","a");  
    fprintf(ptr, debugMessage);
    fclose(ptr);
    
    // if ( (registers.PC == 0xDEFA) || (go == 2)){
    // go++;

    // debugMessageP += sprintf(debugMessageP, "\n\n%s \ninstruction: 0x%02X Operand: 0x%04X \nOPID: %d, cycles: %u", label, instruction, operand, instructions[instruction].operand_length, instructions[instruction].cycles);
	// debugMessageP += sprintf(debugMessageP, "\n\nAF: 0x%04X\n", registers.AF);
	// debugMessageP += sprintf(debugMessageP, "BC: 0x%04X\n", registers.BC);
	// debugMessageP += sprintf(debugMessageP, "DE: 0x%04X\n", registers.DE);
	// debugMessageP += sprintf(debugMessageP, "HL: 0x%04X\n", registers.HL);
	// debugMessageP += sprintf(debugMessageP, "SP: 0x%04X\n", registers.SP);
	// debugMessageP += sprintf(debugMessageP, "PC : 0x%04X\n", registers.PC );
	// debugMessageP += sprintf(debugMessageP, "\nIME: 0x%02X\n", interrupt.master);
	// debugMessageP += sprintf(debugMessageP, "IE: 0x%02X\n", interrupt.enable);
	// debugMessageP += sprintf(debugMessageP, "IF: 0x%02X\n", interrupt.flag);
    // debugMessageP += sprintf(debugMessageP, "LY: %d, LYC: %d, DIV: %d \n", lcd.LY, lcd.LYC, timer.DIV);
    // debugMessageP += sprintf(debugMessageP, "SCY: %d, SCX: %d, WX: %d, WY: %d\n", lcd.SCY, lcd.SCX, lcd.WX, lcd.WY);
    // debugMessageP += sprintf(debugMessageP, "lcdc: %X, stat: %X, Bank: %d\n", lcd.control, lcd.status, currentRomBank);
    // debugMessageP += sprintf(debugMessageP, "TIMA: %04X, TMA: %04X, TAC: %04X\n", timer.TIMA, timer.TMA, timer.TAC);
	// debugMessageP += sprintf(debugMessageP, "scanline_counter: %d, Frame: %d\n", ScanlineCounter, frame);
	// debugMessageP += sprintf(debugMessageP, "\nTry again: LogMemory, continue: +1\n");
	
    // debugMessageP += sprintf(debugMessageP, "Read Stack: %02x %02X", ReadByte(registers.SP), ReadByte(registers.SP + 1));
    // int response = MessageBox(NULL, debugMessage, "Debug Step", MB_DEFBUTTON3 | MB_CANCELTRYCONTINUE | MB_ICONINFORMATION);
    
    // if (response == 2) exit(0);
    // }

    // if (response == 10) { //try again 10 cancel: 2 continue: 11
    //     //Log Memory
    //     LogMemory();

        

    //     // instSkips = 5;
    // } 


}
	

