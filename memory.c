#include <stdio.h>
#include <string.h>


#include "header.h"
#include "memory.h"
#include "control.h"
#include "Debugging.h"
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

char Title[16];

void Reset(void)
{

    memset(VRAM, 0, 0x2000);
    memset(WRAM, 0, 0x2000);
    memset(DisplayPixels, 0, 256 * 256 * 3);

    registers.A = 0x01;
    registers.F = 0xB0; // If the header checksum is $00, then the carry and half-carry flags are clear; otherwise, they are both set.
    registers.B = 0x00; // for the bmg pokemon red start
    registers.C = 0x13;
    registers.D = 0x00;
    registers.E = 0xD8;
    registers.H = 0x01;
    registers.L = 0x4D;
    registers.SP = 0xfffe;
    registers.PC = 0x100;

    joypad.keys = 0xFF;

    interrupt.master = 0;
    interrupt.enable = 0;
    interrupt.flag = 0x01;



    timer.DIV = 0xAB;
    timer.TIMA = 0;
    timer.TMA = 0;
    timer.TAC = 0xF8;
    
    lcd.LY = 0;

    
    // [$FF10] = $80 ; NR10
    // [$FF11] = $BF ; NR11
    // [$FF12] = $F3 ; NR12
    // [$FF14] = $BF ; NR14
    // [$FF16] = $3F ; NR21
    // [$FF17] = $00 ; NR22
    // [$FF19] = $BF ; NR24
    // [$FF1A] = $7F ; NR30
    // [$FF1B] = $FF ; NR31
    // [$FF1C] = $9F ; NR32
    // [$FF1E] = $BF ; NR33
    // [$FF20] = $FF ; NR41
    // [$FF21] = $00 ; NR42
    // [$FF22] = $00 ; NR43
    // [$FF23] = $BF ; NR30
    // [$FF24] = $77 ; NR50
    // [$FF25] = $F3 ; NR51
    // [$FF26] = $F1-GB, $F0-SGB ; NR52
    // AUDIO

    lcd.control = 0x91;
    lcd.status = 0x85;
    lcd.SCY = 0x00;
    lcd.SCX = 0;
    lcd.LYC = 0;
    lcd.WY = 0;
    lcd.WX = 0;

    // WriteByte(0xFF47, 0xFC); // BGP = 0xFF;
    // WriteByte(0xFF48, 0xFF);
    // WriteByte(0xFF49, 0xFF);

   
}

void LogMemory (void) 
{

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

void LoadRomTitle (void)
{
    
    for (int i = 0; i < 0x0010; i++)
    {
        
        Title[i] = ROMBANKS[0x134+i];
        
        
    }
}

void LoadRom (char * Filename)
{

    FILE *f;
    f = fopen(Filename, "r");
    //Reads all x8000 bytes of Rom data
    // fread(ROMBANK0, 0x4000, 1, f);
    fread(ROMBANKS, 0x4000, 64, f);
    printf("Completed Loading Rom: %s \n", Filename);
    LoadRomTitle();
    fclose(f);

    RomLoaded = 1;
}

unsigned char ReadByte (unsigned short Address) 
{

    currentRomBank = ((RomBankHi <<5) | (RomBankLo));
    if (currentRomBank == 0) {currentRomBank++;}

    unsigned char output;

    
    if (Address <= 0x3FFF)
    {

        output = ROMBANKS[Address];

    } else if (Address >= 0x4000 && Address <= 0x7FFF) 
    {
        //if(currentRomBank > 1)printf("reading ROMMBANK: %i\n", currentRomBank);
        output = ROMBANKS[Address - 0x4000 + ((currentRomBank) * 0x4000)]; //0000 is rombank 1
         
    } else if (Address >= 0x8000 && Address <= 0x9FFF) 
    {

        output = VRAM[Address - 0x8000];

    } else if (Address >= 0xA000 && Address <= 0xBFFF) 
    {
        //printf("reading SRAMBANK: %i", currentSRAMBank);

        // if (RAMENABLE) {
            output = SRAMBANKS[Address - 0xA000 + (currentSRAMBank * 0x2000)];
        // } else {
        //     output = SRAMBANKS[Address- 0xA000]; // extra ram not enabled
        // }
        

    } else if (Address >= 0xC000 && Address <= 0xDFFF) 
    {

        output = WRAM[Address - 0xC000];

    } else if (Address >= 0xE000 && Address <= 0xFDFF) 
    { //Echo RAM

        output = WRAM[Address - 0xE000];

    }else if (Address >= 0xFE00 && Address <= 0xFE9F) 
    {
        return OAM[Address - 0xFE00];

    } 
    else if (Address >= 0xFEA0 && Address <= 0xFEFF) 
    {


        return 0;

    } else if (Address >= 0xFF00 && Address < 0xFF80)
    {

        switch(Address){ //IO
        case 0xFF04:

            output = timer.DIV;
            break;
        case 0xFF05:

            output = timer.TIMA;
            break;
        case 0x0FF06:

            output = timer.TMA;
            break;
        case 0xFF07: //tima stacking

            output = timer.TAC;
            break;
        case 0xFF00:
            //printf("reading from keys\n");
            //printf("keys: %02X\n", joypad.keys);
            output = joypad.keys;
        
            break;
        case 0xFF0F: //IF
            
            output = interrupt.flag;
            // printf("Reading from IF, IF: %02X \n", interrupt.flag);
            break;
        case 0xFF40:

            output = lcd.control;
            break;
        case 0xFF41:

            output = lcd.status;
            break;
        case 0xFF42:
            output = lcd.SCY;
            break;
        case 0xFF43:
            output = lcd.SCX;
            break;
        case 0xFF44:
            output = lcd.LY; //lcd.LY stubbing
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
    else if (Address >= 0xFF80 && Address <= 0xFFFF) 
    {

        if(Address == 0xFFFF){
            output = interrupt.enable;
            // printf("Reading from IE, IE: %02X \n", interrupt.enable);
        } else {
            output = HRAM[Address - 0xFF80];
        }

    } 
    
    return output;

}

void WriteByte (unsigned short Address, unsigned char value) 
{

    currentRomBank = ((RomBankHi <<5) | (RomBankLo));
    if (currentRomBank == 0) {currentRomBank++;}


    if (Address <= 0x1FFF)
    { //RAM ENABLE

        value &= 0x0F;
        if (value == 0x0A) {RAMENABLE = 1;} else {RAMENABLE = 0;}

    } else if (Address >= 0x2000 && Address <= 0x3FFF) 
    {
        
        RomBankLo = (value & 0x1F);
        //printf("RomBankLo: %02X from value: %02X\n", RomBankLo, value);
         
    } else if (Address >= 0x4000 && Address <= 0x5FFF) 
    {

        value &= 0x03;

        if(RAMMODE) {

            currentSRAMBank = value;

        } else {

            RomBankHi = value;
            //printf("RomBankHi: %02X from value: %02X\n", RomBankHi, value);
        }

    } else if (Address >= 0x6000 && Address <= 0x7FFF) 
    {

        if(value == 0x00) {
            RAMMODE = 0;
        } else {
            RAMMODE = 1;
        }

    } else if (Address >= 0x8000 && Address <= 0x9FFF) 
    {

        VRAM[Address - 0x8000] = value;

    } else if (Address >= 0xA000 && Address <= 0xBFFF) 
    {
        
        if (RAMENABLE) {SRAMBANKS[Address - 0xA000 + (currentSRAMBank * 0x2000)] = value;}

    } else if (Address >= 0xC000 && Address <= 0xDFFF) 
    {

        WRAM[Address - 0xC000] = value;

    } else if (Address >= 0xE000 && Address <= 0xFDFF) 
    { //Echo RAM

        WRAM[Address - 0xC000] = value;

    } else if (Address >= 0xFE00 && Address < 0xFEA0) 
    {
        OAM[Address - 0xFE00] = value;

    }else if (Address >= 0xFEA0 && Address <= 0xFEFF) 
    {
        return;

    } else if (Address >= 0xFF00 && Address < 0xFF80) 
    {

        switch(Address){
        // case 0xFF02:
        //     //blargg testing writing to Serial Link 
        //     printf("%c", ReadByte(0xFF01));
        //     break;
        // case 0xFF01:
        //     IO[01] = value;
        //     break;
        case 0xFF04:

            timer.DIV = 0;
            break;
        case 0xFF05:

            timer.TIMA = value;
            break;
        case 0x0FF06:

            timer.TMA = value;
            break;
        case 0xFF07: //tima stacking

            int old_freq = timer.TAC & 0x3; //only update if new
            timer.TAC = value;
            int new_freq = timer.TAC & 0x3;

            if (old_freq != new_freq) {timercounter = CLOCKSPEED / GetFrequency();}
            
            break;
        case 0xFF00:

            //printf("writing to keys\n");
            unsigned char temp = joypad.keys;

            //printf("value: %02X\n", value);
            joypad.keys = value;
            joypad.keys &= (0x30);
            joypad.keys |= temp;

            break;
        case 0xFF0F: //IF
            
            interrupt.flag = value;
            // printf("writing to IF, IF: %02X from value: %02X\n", interrupt.flag, value);
            break;
        
        case 0xFF40:

            lcd.control = value;
        case 0xFF41:

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
        case 0xFF46:
            DoDMATransfer(value);
            break;

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
    else if (Address >= 0xFF80 && Address <= 0xFFFF) 
    {

        if (Address == 0xFFFF) 
        {//IE
            
            interrupt.enable = value;
            // printf("writing to IE, IE: %02X from value: %02X\n", interrupt.enable, value);
        }
        else 
        {

            HRAM[Address - 0xFF80] = value;

        }

    }  
    
}

unsigned short ReadShort (unsigned short Address) 
{

    unsigned short output = (unsigned short) ((ReadByte(Address)) | (ReadByte(Address + 1)<<8));
    return output;

}

void WriteShort (unsigned short Address, unsigned short value) 
{
    
    WriteByte(Address, (unsigned char)(value & 0xFF));
    WriteByte(Address + 1, (unsigned char)( (value & 0xFF00) >> 8) );

}

void WriteStack (unsigned short value) 
{

    registers.SP -= 2;
    WriteShort (registers.SP, value);

}

unsigned short ReadStack (void) 
{

    unsigned short output = ReadShort(registers.SP);
    registers.SP += 2;
    return output;

}

void DoDMATransfer(unsigned char Addr)
{
    
    cyclesRegained += 160; //160 machine cycles per DMA
    unsigned short address = Addr << 8 ; // source address is 0xXX00;
    for (int i = 0 ; i < 0xA0; i++)
    {

    WriteByte(0xFE00+i, ReadByte(address+i));

    }

    // for(int i = 0; i < 0xA0; i++)
    // {
    //     printf("OAM[%d] = %d\n", i, ReadByte(0xFE00 + i));
    // }

}