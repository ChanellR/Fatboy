#include <windows.h>
#include <stdio.h>

#include "header.h"
#include "memory.h"
#include "control.h"
#include "display.h"

unsigned short operand = 0;
unsigned char instruction;

int CreateBox(char *label);

int realtimeDebug(void) {
	
	int output;

    CreateBox("before read");
    instruction = ReadByte(registers.PC++);
    CreateBox("after read");

    switch (instructions[instruction].operand_length){
        
        case -4:
            operand = ReadByte(registers.PC++);
            output = CreateBox("after operand (byte)");
            ((void (*)(unsigned char, signed char))instructions[instruction].function)(instruction, (signed char) operand);
            
            
            break;
        case -3:

            operand = ReadShort(registers.PC);
            output = CreateBox("after operand (short)");
            registers.PC += 2;
            ((void (*)(unsigned char, unsigned short))instructions[instruction].function)(instruction, operand);
            CreateBox("after op");
            
            break;

        case -2:
            operand = ReadByte(registers.PC++);
             output = CreateBox("after operand (byte)");
            ((void (*)(unsigned char, unsigned char))instructions[instruction].function)(instruction, operand);
            break;
        case -1:

            output = CreateBox("no operand");
            ((void (*)(unsigned char))instructions[instruction].function)(instruction);
            break;

        case 0:
        output = CreateBox("no op/inst passing");
            ((void (*)())instructions[instruction].function)();
             
            break;

        case 1:
            operand = ReadByte(registers.PC++);
            output = CreateBox("after operand (byte)");
            ((void (*)(unsigned char))instructions[instruction].function)(operand);
             
             
            break;

    }
	
  
	// if(instructions[instruction].operand_length) debugMessageP += 
	// else debugMessageP += sprintf(debugMessageP, instruction);
    

    return output;
}


int CreateBox (char *label){
    char debugMessage[5000];
    char *debugMessageP = debugMessage;

    
    debugMessageP += sprintf(debugMessageP, "%s \ninstruction: 0x%04x Operand: 0x%04x \nOPID: %d, cycles: %u", label, instruction, operand, instructions[instruction].operand_length, instructions[instruction].cycles);
	debugMessageP += sprintf(debugMessageP, "\n\nAF: 0x%04x\n", registers.AF);
	debugMessageP += sprintf(debugMessageP, "BC: 0x%04x\n", registers.BC);
	debugMessageP += sprintf(debugMessageP, "DE: 0x%04x\n", registers.DE);
	debugMessageP += sprintf(debugMessageP, "HL: 0x%04x\n", registers.HL);
	debugMessageP += sprintf(debugMessageP, "SP: 0x%04x\n", registers.SP);
	debugMessageP += sprintf(debugMessageP, "PC : 0x%04x\n", registers.PC );
	debugMessageP += sprintf(debugMessageP, "\nIME: 0x%02x\n", interrupt.master);
	debugMessageP += sprintf(debugMessageP, "IE: 0x%02x\n", interrupt.enable);
	debugMessageP += sprintf(debugMessageP, "IF: 0x%02x\n", interrupt.flag);
	
	debugMessageP += sprintf(debugMessageP, "\nContinue debugging?\n");
	
	return MessageBox(NULL, debugMessage, "Debug Step", MB_YESNO);
}



// void CenterWND (HWND hwnd) {

//     RECT r;
//     GetWindowRect(hwnd, &r);
//     int swidth = GetSystemMetrics(SM_CXSCREEN);
//     int sheight = GetSystemMetrics(SM_CYSCREEN);

//     SetWindowPos(hwnd, NULL, 0, 0, swidth, sheight, SWP_SHOWWINDOW);
// } 

// void DrawImage (HWND hwnd) {

//     HDC hdc;
//     PAINTSTRUCT ps;
//     RECT r;
    
//     GetWindowRect(hwnd, &r);

//     int W = r.right - r.left;
//     int H = r.bottom - r.top;

//     int left_offset = 50;
//     int top_offset = 50;
    
//     int GameWindowWidth = W * .60;
//     int GameWindowHeight = GameWindowWidth;

//     int XPixelCount = 40; //number of pixels across screen
//     int YPixelCount = 40; //number of pixels down screen

//     int RWidth = GameWindowWidth / XPixelCount;
//     int RHeight = GameWindowWidth / YPixelCount;

//     HBRUSH White = GetStockObject(WHITE_BRUSH);
//     HBRUSH Black = GetStockObject(BLACK_BRUSH);

//     hdc = BeginPaint(hwnd, &ps);

//     for (int i = 0; i< XPixelCount; i++) {
//         for (int j = 0; j <YPixelCount; j++ ) {
            
//             if (i % 2 == 1) {
//                 SelectObject(hdc, White);
//             } else {
//                 SelectObject(hdc, Black);
//             }
          
//             Rectangle(hdc, left_offset + (i * RWidth),top_offset + (j * RHeight), left_offset + (i * RWidth + RWidth), top_offset + (j * RHeight + RHeight));
            
//         }
//     }

//     EndPaint(hwnd, &ps);

// }

