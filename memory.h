#pragma once


extern unsigned char VRAM[0x2000]; //8000-9FFF
extern unsigned char ROMBANKS[64 * 0x4000];
extern int currentRomBank;
extern char Title[16];

struct MBC1
{
    unsigned char ramEnable : 4; //1FFF
    unsigned char romBankNumber : 5; //3FFF
    unsigned char ramBankNumber_UpperTwoRom : 2; //5FFF
    unsigned char bankingMode : 1; //7FFF
};

struct MBC3
{
    unsigned char ramEnable : 4;
    unsigned char romBankNumber : 7;
    unsigned char ramBankNumber_RTC : 4;
    //int Latch Clock
};

struct CatridgeHeader 
{
    char title[16];
    int MBCmode;
    int romSize;
    int ramSize;
};

unsigned char ReadByte (unsigned short Address);
void WriteByte (unsigned short Address, unsigned char value);

unsigned short ReadShort (unsigned short Address);
void WriteShort (unsigned short Address, unsigned short value);

unsigned short ReadStack (void);
void WriteStack (unsigned short value);

void DoDMATransfer(unsigned char Addr);
void LoadRomTitle (void);
int DetectMBC (void);

void Save(void);
void Load(void);
void Log(void);
