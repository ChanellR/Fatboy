#pragma once

extern unsigned char DisplayPixels [160 * 144 * 3]; //row, pixel, color
extern int ScanlineCounter;
extern unsigned char SpriteExplorerDisplay [256 * 256 * 3];

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
void LoadTilesFromMap (void);
void LoadLineFromMap (void);
void LoadSpriteLine (void);
void LoadSpriteSheet (void);
void LoadSpritesOnScreen (void);
void LoadOAM (void);


unsigned short GetBGMAPAddress (void);
unsigned short GetDataAddress (void);
