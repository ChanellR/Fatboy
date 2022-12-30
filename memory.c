#include <stdio.h>
#include <string.h>
#include <math.h>

#include "header.h"
#include "memory.h"
#include "control.h"
#include "ppu.h"
#include "apu.h"

#define CLOCKSPEED 4194304

unsigned char ROMBANKS[64 * 0x4000]; //4000 - 7FFF
unsigned char VRAM[0x2000]; //8000-9FFF
unsigned char SRAMBANKS[4 * 0x2000]; //A000-BFFF

unsigned char unusablemem[0xFEFF-0xFEA0 + 1];
unsigned char WRAM[0x2000]; //C000-Dfff
unsigned char OAM[0xA0]; //FE00 - FE9F
unsigned char IO[0x80]; //FF00 - FF7F
unsigned char HRAM[(0xFFFF - 0xFF7F)]; //FF80 - FFFE 
//MAY HAVE GLITCHED HRAM

char Title[16];

int MBCmode;

// struct CatridgeHeader Header;
struct MBC1 MBC1;
struct MBC3 MBC3;

// joypad bits: Start Select B A / Down Up Left Right
unsigned char JoypadState;

int DetectMBC (void) 
{

    //Pokemon, 64 banks 1MB, 4 bank RAM, 32KB
    //Only really care about compatibility with Zelda, and Pokemon, and non MBC
    //Zelda: MBC1 + RAM + Battery, Pokemon: MBC3 + RAM + Battery
    //Pokemon only has 64 banks, Zelda, 32

    int CartNum = (ReadByte(0x0147));
    switch(CartNum)
    {
        case 0:
        case 8: case 9: //Ram and Ram + battery
            MBCmode = 0; //only ROM

            break;
        case 1:case 2:case 3:
            MBCmode = 1;
            break; 
        case 0x0F: case 0x10: case 0x11: case 0x12: case 0x13:
            MBCmode = 3;
            break; 
    }
    
    printf("MBC: %i, ROM: %02X\n", MBCmode, ReadByte(0x0148));
}

void Reset(void)
{
    memset(ROMBANKS, 0, 0x4000 * 64);
    memset(VRAM, 0, 0x2000);
    memset(WRAM, 0, 0x2000);
    memset(SRAMBANKS, 0, 4 * 0x2000);
    memset(OAM, 0, 0xA0);
    memset(HRAM, 0, sizeof(HRAM));
    memset(DisplayPixels, 0, 160 * 144 * 3);

    memset(&MBC1, 0, 2);
    memset(&MBC3, 0, 2);
    // RomBankHi = RomBankLo = currentRomBank = currentSRAMBank = RAMMODE = RAMENABLE = 0;

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
                                registers.SP, registers.PC, lcd.control, lcd.status, lcd.SCX, lcd.SCY, lcd.LYC,
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
    fwrite(&MBC1, 2, 1, ptr);

    fclose(ptr);

}

void Load (void) 
{

    FILE *ptr;
    char SavefileName[20];
    sprintf(SavefileName, "%s.sav", Title);
    ptr = fopen(SavefileName,"rb");  
    
    unsigned short Savedata[13];
                                
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
    fread(&(MBC1), 2, 1, ptr);

    registers.AF = Savedata[0];
    registers.BC = Savedata[1];
    registers.DE = Savedata[2];
    registers.HL = Savedata[3];
    registers.SP = Savedata[4];
    registers.PC = Savedata[5];
    // currentRomBank = Savedata[6];
    // currentSRAMBank = Savedata[7];
    // RAMMODE = Savedata[8];
    // RAMENABLE = Savedata[9];
    lcd.control = Savedata[6];
    lcd.status = Savedata[7];
    lcd.SCX = Savedata[8];
    lcd.SCY = Savedata[9];
    lcd.LYC = Savedata[10];
    lcd.WY = Savedata[11];
    lcd.WX = Savedata[12];

    fclose(ptr);
    
}

void Log(void)
{
    // FILE *ptr;
    // char debugMessage[250];
    // // sprintf(&debugMessage, "A: %02X F: %02X B: %02X C: %02X D: %02X E: %02X H: %02X L: %02X SP: %04X PC: 00:%04X (%02X %02X %02X %02X)\n", 
    // //             registers.A, registers.F, registers.B, registers.C, registers.D, registers.E, registers.H, registers.L, registers.SP, registers.PC, 
    // //             ReadByte(registers.PC), ReadByte(registers.PC + 1), ReadByte(registers.PC + 2), ReadByte(registers.PC + 3));
    // sprintf(&debugMessage, "PC: 00:%04X (%02X %02X %02X %02X)\n", 
    //             registers.PC, ReadByte(registers.PC), ReadByte(registers.PC + 1), ReadByte(registers.PC + 2), ReadByte(registers.PC + 3));
   
    // ptr = fopen("instructionLog.txt","a");  
    // fprintf(ptr, debugMessage);
    // fclose(ptr);
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
    
    // fread(ROMBANKS, 0x4000, 1, f);
    
    // int numOfBanks = 2 * pow(2, ReadByte(0x0148));
    // fread(ROMBANKS, 0x4000, numOfBanks - 1, f);

    
    fread(ROMBANKS, 0x4000, 64, f);
    printf("Completed Loading Rom: %s \n", Filename);
    LoadRomTitle();
    
    fclose(f);

    RomLoaded = 1;
}

int GetRomBank (int romSection)
{
    //romSection 0 = 0000h - 3FFFh
    //romSection 1 = 4000h - 7FFFh
    
    if(MBCmode == 0) return (romSection == 0) ? 0 : 1;

    int output = 0;

    if(MBCmode == 1)
    {
        //will only use lower as to be compatible with Zelda
        output = (romSection == 0) ? 0 : MBC1.romBankNumber;
        
    }
    
    if(MBCmode == 3)
    {
        output = (romSection == 0) ? 0 : MBC3.romBankNumber;   
    }

    return output;
}

int SetMBC (int section, unsigned char value)
{
    //section 0 = 0000h - 1FFFh
    //section 1 = 2000h - 3FFFh
    //section 2 = 4000h - 5FFFh
    //section 3 = 6000h - 7FFFh
    
    if(MBCmode == 1)
    {
        switch(section)
        {
            case 0:
                MBC1.ramEnable = value;
                break;
            case 1:
                MBC1.romBankNumber = value;
                //this gets masked with smaller roms
                break;
            case 2: 
                MBC1.ramBankNumber_UpperTwoRom = value;
                break;
            case 3: 
                MBC1.bankingMode = value;
                break;
        }
    }

    if(MBCmode == 3)
    {
        switch(section)
        {
            case 0:
                MBC3.ramEnable = value;
                break;
            case 1:
                MBC3.romBankNumber = value;
                //this gets masked with smaller roms
                break;
            case 2: 
                MBC3.ramBankNumber_RTC = value;
                break;
            // case 3: 
            //     Latch Clock = value;
            //     break;
        }
    }

}

unsigned char ReadByte (unsigned short Address) 
{

    unsigned char output;

    if (Address <= 0x3FFF)
    {

        output = ROMBANKS[Address + (GetRomBank(0) * 0x4000)];

    } else if (Address >= 0x4000 && Address <= 0x7FFF) 
    {
        
        output = ROMBANKS[Address - 0x4000 + (GetRomBank(1) * 0x4000)]; 
                 
    } else if (Address >= 0x8000 && Address <= 0x9FFF) 
    {

        output = VRAM[Address - 0x8000];

    } else if (Address >= 0xA000 && Address <= 0xBFFF) 
    {
        if(MBCmode == 1)
        {
            if(MBC1.ramEnable == 0x0A)
            {
                // printf("Enable: %02X, Rambank: %02X\n", MBC1.ramEnable, MBC1.ramBankNumber_UpperTwoRom);
                if(MBC1.bankingMode == 0)
                {
                    output = SRAMBANKS[Address - 0xA000]; //set to 0
                }
                else 
                {
                    output = SRAMBANKS[Address - 0xA000 + (MBC1.ramBankNumber_UpperTwoRom * 0x2000)];
                }
                
            } else output = 0xFF; //temp
        }

        if(MBCmode == 3)
        {
            if(MBC3.ramEnable == 0x0A)
            {

                output = SRAMBANKS[Address - 0xA000 + (MBC3.ramBankNumber_RTC * 0x2000)];

            } else output = 0xFF; //temp
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

        SetMBC(0, value);

    } 
    else if (Address >= 0x2000 && Address <= 0x3FFF) 
    {
        
        SetMBC(1, value);
      
    } 
    else if (Address >= 0x4000 && Address <= 0x5FFF) 
    {

        SetMBC(2, value);

    } 
    else if (Address >= 0x6000 && Address <= 0x7FFF) 
    {

        SetMBC(3, value);

    } 
    else if (Address >= 0x8000 && Address <= 0x9FFF) 
    {

        VRAM[Address - 0x8000] = value;

    } 
    else if (Address >= 0xA000 && Address <= 0xBFFF) 
    {
        if(MBCmode == 1)
        {   
            if(MBC1.ramEnable == 0x0A)
            {
                if(MBC1.bankingMode == 0)
                {
                    SRAMBANKS[Address - 0xA000] = value; //set to 0
                }
                else 
                {
                    SRAMBANKS[Address - 0xA000 + (MBC1.ramBankNumber_UpperTwoRom * 0x2000)] = value;
                }
                
            } 
        }

        if(MBCmode == 3)
        {
            if(MBC3.ramEnable == 0x0A)
            {

                SRAMBANKS[Address - 0xA000 + (MBC3.ramBankNumber_RTC * 0x2000)] = value;

            } 
        }

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
            // if(Address == 0xFF14 && ( value & 0x80 ) ) TriggerChannel(1);
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