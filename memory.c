#include <stdio.h>
#include <string.h>


#include "header.h"
#include "memory.h"
#include "control.h"
#include "ppu.h"

#define CLOCKSPEED 4194304


//Just focus on using Boot Rom for now 

// coded for pokemon red MBC3
//64 banks

unsigned char RAMMODE = 0;

// unsigned char ROMBANK0[0x4000];// 0000-3FFF
unsigned char ROMBANKS[129 * 0x4000]; //4000 - 7FFF

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
unsigned long romswitches = 0;

int MBCmode;
char Title[16];


// joypad bits: Start Select B A / Down Up Left Right
unsigned char JoypadState;

int DetectMBC (void) 
{
    
    // MBC1: max 2MB ROM (125 banks) and/or up to 32KB RAM
    // MBC2: max 256KB ROM (16 banks) and 512x4 bits RAM
    // MBC3: max 2MB ROM (128 banks) and/or 32KB RAM (4 banks) and Timer
    // MBC5: max 8MB ROM (512 banks) and/or 128KB RAM (16 banks)
    // HuC1: Similar to MBC1 with an Infrared Controller

    //Pokemon, 64 banks 1MB, 4 bank RAM, 32KB
    printf("MBC: %02X, ROM: %02X\n", ReadByte(0x0147), ReadByte(0x0148));
    int CartType = (ReadByte(0x0147));
    if((CartType >= 0x0F && CartType <= 0x19)){
        MBCmode = 3;
    } else {MBCmode = 1;}
}

void Reset(void)
{

    memset(VRAM, 0, 0x2000);
    memset(WRAM, 0, 0x2000);
    memset(DisplayPixels, 0, 160 * 144 * 3);

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

    WriteByte(0xFF47, 0xFC); 
    WriteByte(0xFF48, 0xFF);
    WriteByte(0xFF49, 0xFF);

   
}

void Save (void) 
{

    FILE *ptr;
    char SavefileName[20];
    sprintf(SavefileName, "%s.sav", Title);
    ptr = fopen(SavefileName,"wb");  
    //AF BC DE HL SP PC
    //rombank srambank rammode ramenable
    unsigned short Savedata[] = {registers.AF, registers.BC, registers.DE, registers.HL,
                                registers.SP, registers.PC, currentRomBank, currentSRAMBank,
                                RAMMODE, RAMENABLE, lcd.control, lcd.status, lcd.SCX, lcd.SCY, lcd.LYC,
                                lcd.WY, lcd.WX};
                                
    fwrite(ROMBANKS, 0x8000, 1, ptr); //ill figure out how to displace it later
    fwrite(VRAM, sizeof(VRAM), 1, ptr);
    fwrite(SRAMBANKS, 0x2000, 1, ptr);
    fwrite(WRAM, sizeof(WRAM), 1, ptr);
    fwrite(WRAM, sizeof(WRAM) - 0x200, 1, ptr);
    fwrite(OAM, sizeof(OAM), 1, ptr);
    fwrite(unusablemem, sizeof(unusablemem), 1, ptr);
    fwrite(IO, sizeof(IO), 1, ptr);
    fwrite(HRAM, sizeof(HRAM), 1, ptr);
    fwrite(Savedata, sizeof(Savedata), 1, ptr);

    fclose(ptr);

}

void Load (void) 
{

    FILE *ptr;
    char SavefileName[20];
    sprintf(SavefileName, "%s.sav", Title);
    ptr = fopen(SavefileName,"rb");  
    
    unsigned short Savedata[17];
                                
    fread(ROMBANKS, 0x8000, 1, ptr); //ill figure out how to displace it later
    fread(VRAM, sizeof(VRAM), 1, ptr);
    fread(SRAMBANKS, 0x2000, 1, ptr);
    fread(WRAM, sizeof(WRAM), 1, ptr);
    fread(WRAM, sizeof(WRAM) - 0x200, 1, ptr);
    fread(OAM, sizeof(OAM), 1, ptr);
    fread(unusablemem, sizeof(unusablemem), 1, ptr);
    fread(IO, sizeof(IO), 1, ptr);
    fread(HRAM, sizeof(HRAM), 1, ptr);
    fread(Savedata, sizeof(Savedata), 1, ptr);

    registers.AF = Savedata[0];
    registers.BC = Savedata[1];
    registers.DE = Savedata[2];
    registers.HL = Savedata[3];
    registers.SP = Savedata[4];
    registers.PC = Savedata[5];
    currentRomBank = Savedata[6];
    currentSRAMBank = Savedata[7];
    RAMMODE = Savedata[8];
    RAMENABLE = Savedata[9];
    lcd.control = Savedata[10];
    lcd.status = Savedata[11];
    lcd.SCX = Savedata[12];
    lcd.SCY = Savedata[13];
    lcd.LYC = Savedata[14];
    lcd.WY = Savedata[15];
    lcd.WX = Savedata[16];

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
    fread(ROMBANKS, 0x4000, 129, f);
    printf("Completed Loading Rom: %s \n", Filename);
    LoadRomTitle();
    fclose(f);

    RomLoaded = 1;
}

unsigned char ReadByte (unsigned short Address) 
{

    currentRomBank = (MBCmode == 1) ? ((RomBankHi <<5) | (RomBankLo)) : currentRomBank;

    unsigned char output;
    //if RAMMODE 1, only rom banks 00-1F can be used but all 4 can be used for mBC1

    
    if (Address <= 0x3FFF)
    {

        output = ROMBANKS[Address];

    } else if (Address >= 0x4000 && Address <= 0x7FFF) 
    {
        
        if(RAMMODE == 1 && MBCmode == 1) currentRomBank & 0x1F;
        if (currentRomBank == 0) currentRomBank = 1;
        int Romoffset = currentRomBank;
        output = ROMBANKS[Address - 0x4000 + ((Romoffset) * 0x4000)]; //0000 is rombank 1
                 
    } else if (Address >= 0x8000 && Address <= 0x9FFF) 
    {

        output = VRAM[Address - 0x8000];

    } else if (Address >= 0xA000 && Address <= 0xBFFF) 
    {
        int SRAMoffset = (RAMMODE == 1) ?  (currentSRAMBank) : 0;

        if (RAMENABLE) {
            output = SRAMBANKS[Address - 0xA000 + (SRAMoffset * 0x2000)];
        } 
        

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
            
            if( !(joypad.keys & 0x10)) {//p14

                output =  ~(JoypadState & 0xF); 

            } 

            if( !(joypad.keys & 0x20))
            {
                output =  ~((JoypadState & 0xF0)>>4);
            }
        
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

    //currentRomBank = (MBCmode != 3) ? ((RomBankHi <<5) | (RomBankLo)) : currentRomBank;


    if (Address <= 0x1FFF)
    { //RAM ENABLE

        value &= 0x0F;
        if (value == 0x0A) 
        {
           // printf("ram enabled");
            RAMENABLE = 1;

        } else 
        {
           //printf("ram disabled");
            RAMENABLE = 0;
            
        }

    } else if (Address >= 0x2000 && Address <= 0x3FFF) 
    {
        
        //MBC3
        
        RomBankLo = (value > 0) ? (value & 0x1F) : 1;
        romswitches++;
        if(MBCmode == 3) currentRomBank = value & 0x7F; 
        
        


        //see if $21 bug is present
        //printf("RomBankLo: %02X from value: %02X\n", RomBankLo, value);
         
    } else if (Address >= 0x4000 && Address <= 0x5FFF) 
    {

        value &= 0x03;
        
        if(RAMMODE) {

            //printf("changing sram value");
            currentSRAMBank = value;

        } else {
            
            RomBankHi = value;
            //printf("RomBankHi: %X, value: %02X\n", RomBankHi, value);
            //printf("RomBankHi: %02X from value: %02X\n", RomBankHi, value);
        }

    } else if (Address >= 0x6000 && Address <= 0x7FFF) 
    {

        if(value == 0x00) {
            //printf("ROM MODE");
            RAMMODE = 0;
        } else if(value == 0x01)
        {
            //printf("ram mode");
            RAMMODE = 1; //ram mode centered 
            // 16Mb ROM/8KB RAM and 4Mb ROM/32KB on MBC1
        }

    } else if (Address >= 0x8000 && Address <= 0x9FFF) 
    {

        VRAM[Address - 0x8000] = value;

    } else if (Address >= 0xA000 && Address <= 0xBFFF) 
    {
        int SRAMoffset = (RAMMODE) ? (currentSRAMBank) : 0;
        if (RAMENABLE) {SRAMBANKS[Address - 0xA000 + (SRAMoffset * 0x2000)] = value;}

    } else if (Address >= 0xC000 && Address <= 0xDFFF) 
    {

        WRAM[Address - 0xC000] = value;

    } else if (Address >= 0xE000 && Address <= 0xFDFF) 
    { //Echo RAM

        WRAM[Address - 0xC000] = value;

    } else if (Address >= 0xFE00 && Address <= 0xFE9F) 
    {
        if((lcd.status & 0x03) < 2) OAM[Address - 0xFE00] = value;

    } else if (Address >= 0xFEA0 && Address <= 0xFEFF) 
    {
        return;

    } else if (Address >= 0xFF00 && Address <= 0xFF7F) 
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
            timer.TIMA = timer.TMA;
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
            printf("TAC: %02X\n", timer.TAC);
            int new_freq = timer.TAC & 0x3;
            if (old_freq != new_freq) {timercounter = CLOCKSPEED / GetFrequency();}
            break;
        case 0xFF00:
            joypad.keys = value;
            break;
        case 0xFF0F: //IF
            interrupt.flag = value;//see if working
            break;
        case 0xFF40:
            lcd.control = value;
            break;
        case 0xFF41:
            lcd.status = value;
            break;
        case 0xFF42:
            lcd.SCY = value;
            break;
        case 0xFF43:
            lcd.SCX = value;
            break;
        case 0xFF44:
            lcd.LY = value; //reset cirumcumstantial change
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
    //see if there are timing issues, what exactly is happening during the opening
    //movie that makes it so scuffed for sprites. 

    cyclesRegained += 160; //160 machine cycles per DMA
    unsigned short address = Addr << 8 ; // source address is 0xXX00;
    for (int i = 0 ; i < 0xA0; i++)
    {
        WriteByte(0xFE00+i, ReadByte(address+i));
    }

}