#include <stdio.h>

#include "header.h"
#include "memory.h"
#include "control.h"
#include "display.h"
#include "gpu.h"

#define CLOCKSPEED 4194304


//Just focus on using Boot Rom for now 

// coded for pokemon red MBC3
//64 banks

unsigned char RAMMODE = 0;

// unsigned char ROMBANK0[0x4000];// 0000-3FFF
unsigned char ROMBANKS[64 * 0x4000]; //4000 - 7FFF

unsigned char RomBankLo = 0x01;
unsigned char RomBankHi = 0x0;
unsigned char currentRomBank;

unsigned char VRAM[0x2000]; //8000-9FFF
//A000-BFFF
unsigned char SRAMBANKS[4 * 0x2000];
unsigned char currentSRAMBank = 0;
unsigned char RAMENABLE = 0;

unsigned char unusablemem[0xFEFF-0xFEA0 + 1];
unsigned char WRAM[0x2000]; //C000-Dfff
unsigned char OAM[0xA0]; //FE00 - FE9F
unsigned char IO[0x80]; //FF00 - FF7F
unsigned char HRAM[(0xFFFE - 0xFF7F + 1)]; //FF80 - FFFE

char Filename[] = "src/11-op a,(hl).gb";

int skipping = 1;

void LogMemory (void) {

    FILE *ptr;
    ptr = fopen("MemoryLog.bin","wb");  
    // fwrite(ROMBANK0, sizeof(ROMBANK0), 1, ptr);
    fwrite(ROMBANKS, 0x8000, 1, ptr); //ill figure out how to displace it later
    fwrite(VRAM, sizeof(VRAM), 1, ptr);
    fwrite(SRAMBANKS, 0x2000, 1, ptr);
    fwrite(WRAM, sizeof(WRAM), 1, ptr);
    fwrite(WRAM, sizeof(WRAM) - 0x200, 1, ptr);
    fwrite(OAM, sizeof(OAM), 1, ptr);
    fwrite(unusablemem, sizeof(unusablemem), 1, ptr);
    fwrite(IO, sizeof(IO), 1, ptr);
    fwrite(HRAM, sizeof(HRAM), 1, ptr);

    fclose(ptr);
    
}

void LoadRom (void){

    FILE *f;
    f = fopen(Filename, "r");
    //Reads all x8000 bytes of Rom data
    // fread(ROMBANK0, 0x4000, 1, f);
    fread(ROMBANKS, 0x4000, 64, f);

    printf("Completed Loading Rom \n");
    fclose(f);
}

unsigned char ReadByte (unsigned short Address) {

    currentRomBank = ((RomBankHi <<5) | (RomBankLo));
    if (currentRomBank == 0) {currentRomBank++;}

    unsigned char output;

    
    if (Address <= 0x3FFF){

        output = ROMBANKS[Address];

    } else if (Address >= 0x4000 && Address <= 0x7FFF) {

        output = ROMBANKS[Address - 0x4000 + ((currentRomBank) * 0x4000)]; //0000 is rombank 1
         
    } else if (Address >= 0x8000 && Address <= 0x9FFF) {

        output = VRAM[Address - 0x8000];

    } else if (Address >= 0xA000 && Address <= 0xBFFF) {

        // if (RAMENABLE) {
            output = SRAMBANKS[Address - 0xA000 + (currentSRAMBank * 0x2000)];
        // } else {
        //     output = SRAMBANKS[Address- 0xA000]; // extra ram not enabled
        // }
        

    } else if (Address >= 0xC000 && Address <= 0xDFFF) {

        output = WRAM[Address - 0xC000];

    } else if (Address >= 0xE000 && Address <= 0xFDFF) { //Echo RAM

        output = WRAM[Address - 0xE000];

    } else if (Address >= 0xFEA0 && Address <= 0xFEFF) {

        CreateBox("non usable space");
        return 0;

    } else if (Address >= 0xFF00 && Address < 0xFF80){

        switch(Address){ //IO
        case 0xFF04:
            CreateBox("Read from DIV"); 
            output = timer.DIV;
            break;
        case 0xFF05:
            CreateBox("Read from TIMA");
            output = timer.TIMA;
            break;
        case 0x0FF06:
            CreateBox("Read from TMA");
            output = timer.TMA;
            break;
        case 0xFF07: //tima stacking
            CreateBox("Read from TAC");
            output = timer.TAC;
            break;
        case 0xFF00:
            CreateBox("Read from joypad");
            output = joypad.keys;
            break;
        case 0xFF0F: //IF
            
            output = interrupt.flag;
            // printf("Reading from IF, IF: %02X \n", interrupt.flag);
            break;
        case 0xFF40:
            printf("yes");
            CreateBox("Read from lcd.control");
            output = lcd.control;
            break;
        case 0xFF41:
            CreateBox("Read from lcd.status");
            output = lcd.status;
            break;
        case 0xFF42:
            output = lcd.SCY;
            break;
        case 0xFF43:
            output = lcd.SCX;
            break;
        case 0xFF44:
            output = 0x90; //lcd.LY stubbing
            break;
        case 0xFF45:
            output = lcd.LYC;
            break;
        case 0xFF4A:
            output = lcd.WY;
            break;
        case 0xFF4B:
            output = lcd.WX;
            break;       
        default:
            output = IO[Address-0xFF00];
    }
    }
    else if (Address >= 0xFF80 && Address <= 0xFFFF) {

        
        if(Address == 0xFFFF){
            output = interrupt.enable;
            // printf("Reading from IE, IE: %02X \n", interrupt.enable);
        } else {
            output = HRAM[Address - 0xFF80];
        }
    } 
    


    
    // char label[50];
    // sprintf(label, "\nRead byte 0x%02x from (0x%04x)", output, Address);
    // FILE *ptr;
    // ptr = fopen("Log.txt","a");  
    // fprintf(ptr, label);
    // fclose(ptr);
 
    return output;

}

void WriteByte (unsigned short Address, unsigned char value) {

    currentRomBank = ((RomBankHi <<5) | (RomBankLo));
    if (currentRomBank == 0) {currentRomBank++;}

    if (Address <= 0x1FFF){ //RAM ENABLE

        value &= 0x0F;
        if (value == 0x0A) {RAMENABLE = 1;} else {RAMENABLE = 0;}

    } else if (Address >= 0x2000 && Address <= 0x3FFF) {

        RomBankLo = (value & 0x1F);
         
    } else if (Address >= 0x4000 && Address <= 0x5FFF) {

        value &= 0x03;

        if(RAMMODE) {

            currentSRAMBank = value;

        } else {

            RomBankHi = value;
        }

    } else if (Address >= 0x6000 && Address <= 0x7FFF) {

        if(value == 0x00) {
            RAMMODE = 0;
        } else {
            RAMMODE = 1;
        }

    } else if (Address >= 0x8000 && Address <= 0x9FFF) {

        VRAM[Address - 0x8000] = value;

    } else if (Address >= 0xA000 && Address <= 0xBFFF) {

        if (RAMENABLE) {SRAMBANKS[Address - 0xA000 + (currentSRAMBank * 0x2000)] = value;}

    } else if (Address >= 0xC000 && Address <= 0xDFFF) {

        WRAM[Address - 0xC000] = value;

    } else if (Address >= 0xE000 && Address <= 0xFDFF) { //Echo RAM

        WRAM[Address - 0xC000] = value;

    } else if (Address >= 0xFEA0 && Address <= 0xFEFF) {

        CreateBox("non usable space");
        return;

    } else if (Address >= 0xFF00 && Address < 0xFF80) {

        switch(Address){
        case 0xFF02:
            //blargg testing writing to Serial Link 
            printf("%c", ReadByte(0xFF01));
            break;
        case 0xFF01:
            IO[01] = value;
            break;
        case 0xFF04:
            CreateBox("Write to DIV");
            timer.DIV = 0;
            break;
        case 0xFF05:
            CreateBox("Write to TIMA");
            timer.TIMA = value;
            break;
        case 0x0FF06:
            CreateBox("Write to TMA");
            timer.TMA = value;
            break;
        case 0xFF07: //tima stacking
         CreateBox("Write to TAC");
            timer.TAC = value;
            m_timercounter = CLOCKSPEED / GetFrequency();
            break;
        case 0xFF00:
            CreateBox("Write to joypad");
            joypad.keys = value;
            break;
        case 0xFF0F: //IF
            
            interrupt.flag = value;
            // printf("writing to IF, IF: %02X from value: %02X\n", interrupt.flag, value);
            break;
        
        case 0xFF40:
            CreateBox("Write to lcd.control");
            lcd.control = value;
        case 0xFF41:
            CreateBox("Write to lcd.status");
            lcd.status = 0x80;

        case 0xFF42:
            lcd.SCY = value;
            break;
        case 0xFF43:
            lcd.SCX = value;
            break;
        case 0xFF44:
            lcd.LY = 0; //reset cirumcumstantial change
            break;
        case 0xFF45:
            lcd.LYC = value;
        case 0xFF4A:
            lcd.WY = value;
            break;
        case 0xFF4B:
            lcd.WX = value;
            break;  
        default:
            IO[Address-0xFF00] = value;
    } 

    }
    else if (Address >= 0xFF80 && Address <= 0xFFFF) {

        if (Address == 0xFF46)
        {
            DoDMATransfer(value);

        }else if(Address == 0xFFFF) 
        {//IE
            
            interrupt.enable = value;
            // printf("writing to IE, IE: %02X from value: %02X\n", interrupt.enable, value);
        }
        else 
        {

            HRAM[Address - 0xFF80] = value;
        }



    }  

   


    // char label[50]; //labels might mess things up
    // sprintf(label, "\nbyte (0x%04x) = 0x%02x", Address, value);
    // FILE *ptr;
    // ptr = fopen("Log.txt","a");  
    // fprintf(ptr, label);
    // fclose(ptr);
    
    
}

unsigned short ReadShort (unsigned short Address) {
    unsigned short output = (unsigned short) ((ReadByte(Address)) | (ReadByte(Address + 1)<<8));
    // unsigned short output = (unsigned short)( memory[Address] | (memory[Address + 1]<<8) );
    char label[50];
    sprintf(label, "Read short 0x%04x from (0x%04x)", output,  Address);
    CreateBox(label);
    return output;
}


// unsigned short ReadShort (unsigned short Address) {
//     return (unsigned short)( memory[Address] | (memory[Address + 1]<<8) );
// }


void WriteShort (unsigned short Address, unsigned short value) {
    
    WriteByte(Address, (unsigned char)(value & 0xFF));
    WriteByte(Address + 1, (unsigned char)( (value & 0xFF00) >> 8) );
    char label[50];
    sprintf(label, "short (0x%04x + 1) = 0x%04x", Address, value);
    CreateBox(label);
}

void WriteStack (unsigned short value) {

    registers.SP -= 2;
    WriteShort (registers.SP, value);
    char label[50];
    sprintf(label, "Wrote to stack: 0x%04x", value);
    CreateBox(label);
    
}

unsigned short ReadStack (void) {
    unsigned short output = ReadShort(registers.SP);
    registers.SP += 2;
    char label[50];
    sprintf(label, "Read stack: 0x%04x", output);
    CreateBox(label);
    return output;
}

void DoDMATransfer(unsigned char Addr)
{
   unsigned short address = Addr << 8 ; // source address is data * 100
   for (int i = 0 ; i < 0xA0; i++)
   {
    
     WriteByte(0xFE00+i, ReadByte(address+i));

   }
   CreateBox("DMA Transfer");
}