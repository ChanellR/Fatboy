#pragma once


extern unsigned char VRAM[0x2000]; //8000-9FFF
extern unsigned char ROMBANKS[129 * 0x4000];
extern unsigned char currentRomBank;
extern char Title[16];

unsigned char ReadByte (unsigned short Address);
void WriteByte (unsigned short Address, unsigned char value);

unsigned short ReadShort (unsigned short Address);
void WriteShort (unsigned short Address, unsigned short value);

unsigned short ReadStack (void);
void WriteStack (unsigned short value);

void LogMemory (void);
void DoDMATransfer(unsigned char Addr);
void LoadRomTitle (void);
int DetectMBC (void);

void Save(void);
void Load(void);
