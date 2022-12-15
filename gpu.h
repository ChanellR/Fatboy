#pragma once

extern unsigned char Pixels [160 * 144 * 3]; //row, pixel, color
extern int ScanlineCounter;

struct LCD {
    unsigned char control; //FF40
    unsigned char status; //FF41
    unsigned char SCY;
    unsigned char SCX; //top left of Viewport from BG
    unsigned char LY; //Ly FF44
    unsigned char LYC; //LY compare
    unsigned char WY;  //Y position 
    unsigned char WX; //X position plus 7
}extern lcd;

void UpdateGraphics (int cycles);
int IsLcdOn (void);
void SetLCDstatus(void);
void DrawScanline(void);
unsigned char * LoadNintendoLogo (void);
unsigned char * LoadTilesFromMap (unsigned short MapAddress, unsigned short DataAddress);
void RenderSprites(void);


