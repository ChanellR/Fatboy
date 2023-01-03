#include <stdio.h>
#include <string.h>

#include "header.h"
#include "memory.h"
#include "control.h"
#include "ppu.h"

 //every dot and their hue

struct LCD lcd;
int ScanlineCounter; //456 cycles per scan line
unsigned char DisplayPixels [160 * 144 * 3];
unsigned char SpriteExplorerDisplay [256 * 256 * 3];
unsigned char WindowXFlag = 0;
unsigned char WindowYFlag = 0;
int WindowLineCounter = 0;

unsigned char* GetPixelColor (unsigned char bit1, unsigned char bit2, int pallette);

unsigned short GetBGMAPAddress (void) 
{
    unsigned short address = (lcd.control & 0x08) ? 0x9C00 : 0x9800;
    return address;
}

unsigned short GetDataAddress (void) 
{
    unsigned short address = (lcd.control & 0x10) ? 0x8000 : 0x8800;
    return address;
}

unsigned char UsingWindow (void)
{
    return (lcd.control & 0x20);
}

unsigned short GetWINMAPAddress(void) 
{
    unsigned short address = (lcd.control & 0x40) ? 0x9C00 : 0x9800;
    return address;
}

void UpdateGraphics (int cycles) {

    SetLCDstatus();

    if(IsLcdOn()) {
        ScanlineCounter -= (cycles);
    } else {
        return;
    }
    
    if (ScanlineCounter <= 0) {

        
        int overflow = (-1 * ScanlineCounter);
        ScanlineCounter = 456;
        ScanlineCounter -= overflow;
        lcd.LY++;
        

        if(lcd.LY == 144) {
            
            RequestInterrupt(0);
            
        } else if (lcd.LY > 153) {
            
            lcd.LY = 0;
            DrawScanline();

        } else if (lcd.LY < 144) {
            
            DrawScanline(); //this will add a line to the render buffer

        }

    } 

}

int IsLcdOn (void) {
    return (lcd.control & 0x80);
}

void SetLCDstatus (void) {
    //leave the 7th bit
    if (!IsLcdOn()) {

        //lcd off
        ScanlineCounter = 456;
        lcd.LY = 0;
        lcd.status &= 0xFC; //reset mode to 1 Vblank when off
        lcd.status |= 0x01;
        return;

    } 

    unsigned char currentMode = (lcd.status & 0x03);

    unsigned char mode = 0;
    unsigned char ReqInt = 0;

    if (lcd.LY >= 144) {
        
        mode = 1;
        lcd.status &= 0xFC;
        lcd.status |= 0x01;
        ReqInt = (lcd.status & 0x10); // bit 4

    } else if (ScanlineCounter)    
    { //to buffer in the beginning 

        int mode2bounds = 456-80 ;
        int mode3bounds = mode2bounds - 172 ;

        if(ScanlineCounter >= mode2bounds ) {

            mode = 2;
            lcd.status &= 0xFC;
            lcd.status |= 0x02; //searching sprite attributes
            ReqInt = (lcd.status & 0x20); //bit 5

        } else if (ScanlineCounter >= mode3bounds) {

            mode = 3;
            lcd.status &= 0xFC; //transfering data to LCD driver
            lcd.status |= 0x03;
            

        } else {

            mode = 0;
            lcd.status &= 0xFC;
            ReqInt = (lcd.status & 0x08); //Hblank

        }

    }

    if (ReqInt && (currentMode != mode)) {
        RequestInterrupt(1); //stat interrupt
    }

    if (lcd.LY == lcd.LYC) {
        
        lcd.status |= 0x04;

        if(lcd.status & 0x40) { //bit 6
            RequestInterrupt(1);
    }

    } else {
        lcd.status &= ~(0x04); //everything but second bit
    }

}

void DrawScanline(void) {

    if(lcd.control & 0x80) {
        
        LoadLineFromMap();

    } 
    
    if (lcd.control & 0x02) {

        // LoadSpritesOnScreen();
        LoadSpriteLine();

    }
}

void LoadSpritesOnScreen (void) {
    //int UnderBGWIN = lcd.control & 0x01;
    unsigned char SpriteYPos = 0;
    unsigned char SpriteXpos = 0;
    unsigned short DataAddress = 0x8000; //sprites stored 0x8000 - 0x8FFF;
    unsigned char SpriteHeight = (lcd.control & 0x04) ? 16 : 8; //double or not
    int spritesloaded = 0;
    //works but doesn't have attributes nor proper priority;

    for (int SpritetoLoad = 0; SpritetoLoad < 40; SpritetoLoad++) 
    {
        //if(SpritetoLoad == 0) printf("Sprite: %d, ID: %02X, Xcoord: %d, Ycoord: %d Attribute: %02X\n", SpritetoLoad, ReadByte(0xFE00 + SpritetoLoad * 4 + 2), ReadByte(0xFE00 + SpritetoLoad * 4 + 1), ReadByte(0xFE00 + SpritetoLoad * 4 ), ReadByte(0xFE00 + SpritetoLoad * 4 +3));
        
        SpriteYPos = ReadByte(0xFE00 + SpritetoLoad * 4);
        if(SpriteYPos == 0) continue;

        SpriteXpos = ReadByte(0xFE00 + SpritetoLoad * 4 + 1);
        
        int SpritePixelOffset = (SpriteYPos > 15) ? (SpriteXpos - 8) * 3 + (SpriteYPos - 16) * 160 * 3 : (SpriteXpos - 8) * 3;

        unsigned short SpriteDataAddress = DataAddress;
        
        int offset = ReadByte(0xFE00 + SpritetoLoad * 4 + 2);
        SpriteDataAddress += (offset * 16);

        int SpriteStartingLine = (SpriteYPos > 15) ? 0 : -1 * (SpriteYPos - 16); //what line of sprite do i draw first
        
        //Attributes
        int SpriteVMirror = ReadByte(0xFE00 + SpritetoLoad * 4 + 3) & 0x40;
        int SpriteHMirror = ReadByte(0xFE00 + SpritetoLoad * 4 + 3) & 0x20;
        int SpritePallette = ReadByte(0xFE00 + SpritetoLoad * 4 + 3) & 0x10;
        //BG override
        SpritePallette = (ReadByte(0xFE00 + SpritetoLoad * 4 + 3) & 0x80) ? 2: SpritePallette;

        for(int lineofSprite = SpriteStartingLine; lineofSprite < SpriteHeight; lineofSprite++)
        {
            unsigned char byte1;
            unsigned char byte2;

            int lineoffset = (lineofSprite) * (160) * 3;// should start printing only @lcd.LY independent of positon of lcd.SCY

            if(SpriteVMirror){
                    
                    byte1 = ReadByte( SpriteDataAddress + (SpriteHeight - lineofSprite) * 2); //off set by their position in the tile
                    byte2 = ReadByte( SpriteDataAddress + (SpriteHeight - lineofSprite) * 2 + 1);

            } else {

                    byte1 = ReadByte( SpriteDataAddress + lineofSprite * 2); //off set by their position in the tile
                    byte2 = ReadByte( SpriteDataAddress + lineofSprite * 2 + 1);

            }

            for(int pixel = 0; pixel < 8; pixel++)
            {
                unsigned char bit1;
                unsigned char bit2;

                int PixelOffset = pixel * 3;

                if(SpriteHMirror){
                    
                    bit1 = (byte1 & (0x01 << pixel));
                    bit2 = (byte2 & (0x01 << pixel));

                } else {

                    bit1 = (byte1 & (0x80 >> pixel));
                    bit2 = (byte2 & (0x80 >> pixel));

                }


                unsigned char *colors;
                if(!strcmp(Title, "DR.MARIO")) SpritePallette = 2;
                colors = GetPixelColor(bit1, bit2, SpritePallette);

                //00 means transparent, so no write
                if(bit1 || bit2){

                    DisplayPixels[SpritePixelOffset + lineoffset + PixelOffset] = colors[0];
                    DisplayPixels[SpritePixelOffset + lineoffset + PixelOffset + 1] = colors[1];
                    DisplayPixels[SpritePixelOffset + lineoffset + PixelOffset + 2] = colors[2];

                }
            }
        }
        
    }

}

void LoadSpriteLine (void)
{
    //if(lcd.status & 0x03 != 1) return;
    unsigned short DataAddress = 0x8000;
    unsigned short OAMAddress = 0xFE00;

    int SpritesinLine = 0;
    int SpriteHeight = (lcd.control & 0x04) ? 16 : 8;

    for (int Sprite = 0; Sprite < 40; Sprite++)
    {
        if(SpritesinLine == 10) break;

        int SpriteYpos = ReadByte(OAMAddress + Sprite * 4) - 16;

        if(SpriteYpos == -16 || SpriteYpos == 144) continue;
        if(!(lcd.LY >= SpriteYpos && lcd.LY < (SpriteYpos + SpriteHeight))) continue;

        int SpriteXpos = ReadByte(OAMAddress + Sprite * 4 + 1) - 8;
        if (SpriteXpos == -8) {SpritesinLine++; continue;}

        int SpriteID = ReadByte(OAMAddress + Sprite * 4 + 2);
        //printf("Sprite#: %d, Y: %d, X: %d, ID: %02X\n", Sprite, SpriteYpos, SpriteXpos, SpriteID);
        
        //Attributes
        int BGoverOBJ = ReadByte(OAMAddress + Sprite * 4 + 3) & 0x80;
        int SpriteVMirror = ReadByte(OAMAddress + Sprite * 4 + 3) & 0x40;
        int SpriteHMirror = ReadByte(OAMAddress + Sprite * 4 + 3) & 0x20;
        int SpritePallette = (ReadByte(OAMAddress + Sprite * 4 + 3) & 0x10) >> 4;//non cgb
        int SecondSpritePallette = (ReadByte(OAMAddress + (Sprite + 1) * 4  + 3) & 0x10) >> 4; //for second sprite in double length

        unsigned char byte1;
        unsigned char byte2;

        if(SpriteVMirror){

            byte1 = ReadByte( DataAddress + SpriteID * 16 + (SpriteHeight - (lcd.LY - SpriteYpos)) * 2); //off set by their position in the tile
            byte2 = ReadByte( DataAddress + SpriteID * 16 + (SpriteHeight - (lcd.LY - SpriteYpos)) * 2 + 1);

        } else {

            byte1 = ReadByte( DataAddress + SpriteID * 16 + (lcd.LY - SpriteYpos) * 2); //off set by their position in the tile
            byte2 = ReadByte( DataAddress + SpriteID * 16 + (lcd.LY - SpriteYpos) * 2 + 1);

        }

        for (int pixel = 0; pixel < 8; pixel++)
        {   
            //if((lcd.LY - SpriteYpos) > 7) SpritePallette = SecondSpritePallette;
            if(SpriteXpos < 0) pixel += -1 * SpriteXpos;

            unsigned char bit1;
            unsigned char bit2;

            if (SpriteHMirror) 
            {
                
                bit1 = (byte1 & (0x01 << pixel));
                bit2 = (byte2 & (0x01 << pixel));

            }
            else
            {
                bit1 = (byte1 & (0x80 >> pixel));
                bit2 = (byte2 & (0x80 >> pixel));
            }
            
            unsigned char *colors;
            colors = GetPixelColor(bit1, bit2, SpritePallette);

            //00 means transparent, so no write
            if(bit1 || bit2){

                //BG priority
                if(BGoverOBJ && DisplayPixels[lcd.LY * 160 * 3 + SpriteXpos * 3 + pixel * 3] != 0xE0) continue;
                
                DisplayPixels[lcd.LY * 160 * 3 + SpriteXpos * 3 + pixel * 3] = colors[0];
                DisplayPixels[lcd.LY * 160 * 3 + SpriteXpos * 3 + pixel * 3 + 1] = colors[1];
                DisplayPixels[lcd.LY * 160 * 3 + SpriteXpos * 3 + pixel * 3 + 2] = colors[2];

            }

        }


    }
}

unsigned char * LoadNintendoLogo (void){

    static unsigned char PixelBatch[6 * 8 * 8 * 3];

    for (int tile = 0; tile < 48; tile++ )
    {
        int tileOffset = (tile % 2) ? ((tile % 2) * 48 * 3 * 2 + (tile/2 * 4 * 3)) : (tile/2 * 4 * 3); // if odd, skip two lines
        
        tileOffset += (tile >= 24) ? 48 * 3 * 3 : 0; //next line
        unsigned char byte = ReadByte(0x0104 + tile);

        for (int i = 0; i < 8; i++)
        {

        int offset = (i > 3) ? i * 3 + (44 * 3) : i * 3;
        unsigned char color = (byte & (0x80>>i)) ? 0x0 : 0xFF;

        DisplayPixels[offset + tileOffset] = color;
        DisplayPixels[offset + tileOffset + 1] = color;
        DisplayPixels[offset + tileOffset + 2] = color;
        
        }
    }

    return PixelBatch;
}

void PrintOAM (void) 
{

    for (int i = 0; i < 40; i++)
    {
        if(ReadByte(0xFE00 + i * 4 + 2) == 0) continue;
        printf("OAM[%i]: %02X %02X %02X %02X | ", i, 
        ReadByte(0xFE00 + i * 4), ReadByte(0xFE00 + i * 4 + 1), 
        ReadByte(0xFE00 + i * 4 + 2), ReadByte(0xFE00 + i * 4 + 3));

        if(!(i % 2)) printf("\n");
    }

}

void LoadOAM (void) {

    //PrintOAM();

    unsigned short DataAddress = 0x8000;
    int SpriteLength = (lcd.control & 0x04) ? 16 : 8; 

    for (int SpritetoLoad = 0; SpritetoLoad < 40; SpritetoLoad++) 
    {
        //sprite should be spaced 
        int SpritePixelOffset =  10 * 3 + 256 * 3 * 10 + ( (SpritetoLoad / 4) * 256 * 15 + (SpritetoLoad % 4) * 45) * 3;
        int offset = ReadByte(0xFE00 + SpritetoLoad * 4 + 2);
        unsigned short SpriteDataAddress = DataAddress + (offset * 16);
        
        int SpritePallette = ReadByte(0xFE00 + SpritetoLoad * 4 + 3) & 0x10;

        for(int lineofSprite = 0; lineofSprite < SpriteLength; lineofSprite++)
        {
            int lineoffset = lineofSprite * 256 * 3; //move to beginning of line
            unsigned char byte1 = ReadByte( SpriteDataAddress + lineofSprite * 2); //off set by their position in the tile
            unsigned char byte2 = ReadByte( SpriteDataAddress + lineofSprite * 2 + 1);

            for(int pixel = 0; pixel < 8; pixel++)
            {
                
                int PixelOffset = pixel * 3;

                unsigned char bit1 = (byte1 & (0x80 >> pixel));
                unsigned char bit2 = (byte2 & (0x80 >> pixel));

                unsigned char *colors;
                colors =  GetPixelColor(bit1, bit2, SpritePallette);

                //00 means transparent, so no write
                if(bit1 || bit2){

                    SpriteExplorerDisplay[SpritePixelOffset + lineoffset + PixelOffset] = colors[0];
                    SpriteExplorerDisplay[SpritePixelOffset + lineoffset + PixelOffset + 1] = colors[1];
                    SpriteExplorerDisplay[SpritePixelOffset + lineoffset + PixelOffset + 2] = colors[2];

                }
            
            
            }
        }
        
    }
}

void LoadSpriteSheet (void)
{   
    //displays all tiles currently mapped in the BG, doesn't hold scanline functionality but makes sense. 
    //returns 8 x 8 pixel bitmap
    //unsigned only 0x8000-

    unsigned short DataAddress = 0x8000;
    
    for (int tile = 0; tile < ( 32 * 12 ); tile++)
    {
        int tileOffset = (tile % 16) * 8 * 3 + ((tile / 16) * 256 * 3 * 8); //row then coloumn
        //if(tile % 32)
        for (int line = 0; line < 8; line++)
        {
            int lineoffset =  line * (256) * 3; 

            unsigned char byte1 = ReadByte( DataAddress + line * 2 ); //off set by their position in the tile
            unsigned char byte2 = ReadByte( DataAddress + line * 2 + 1 );

            for(int pixel = 0; pixel < 8; pixel++)
            {
                
                int PixelOffset = pixel * 3;

                unsigned char bit1 = (byte1 & (0x80 >> pixel));
                unsigned char bit2 = (byte2 & (0x80 >> pixel));

                unsigned char *colors;
                int pallette = (tile < 16 * 8) ? 0 : 2;
                colors = GetPixelColor(bit1, bit2, pallette);

                SpriteExplorerDisplay[tileOffset + lineoffset + PixelOffset] = colors[0];
                SpriteExplorerDisplay[tileOffset + lineoffset + PixelOffset + 1] = colors[1];
                SpriteExplorerDisplay[tileOffset + lineoffset + PixelOffset + 2] = colors[2];
  
                
            }
        }

        DataAddress += 16; //each tile move forward 16 bytes
    }


}

void LoadTilesFromMap (void)
{   

    //displays all tiles currently mapped in the BG, doesn't hold scanline functionality but makes sense. 
    //returns 8 x 8 pixel bitmap
    //unsigned only 0x8000-

    //printf("SCX: %02X\n", lcd.SCX);
    //printf("SCY: %02X\n", lcd.SCY);

    unsigned short MapAddress = (lcd.control) ? GetBGMAPAddress() : 0x9800; 
    unsigned short DataAddress = (lcd.control) ? GetDataAddress() : 0x8000;
    
    unsigned short TileIDAddress = MapAddress;
    //BG
    
    for (int tile = 0; tile < ( 32 * 32 ); tile++)
    {
        
        int tileOffset = (tile % 32) * 8 * 3 + ((tile / 32) * 256 * 3 * 8); //row then coloumn

        int TileID = (DataAddress == 0x8000) ? (unsigned char) ReadByte(TileIDAddress + tile) : (signed char) ReadByte(TileIDAddress + tile);
        unsigned short TileDataAddress = DataAddress;

        int offset = (DataAddress == 0x8000) ? (TileID * 16) : ((TileID + 128) * 16);
        TileDataAddress += (offset); 

        for (int line = 0; line < 8; line++)
        {
            int lineoffset = line * (256) * 3;// moves back to first spot

            unsigned char byte1 = ReadByte( TileDataAddress + line * 2); //off set by their position in the tile
            unsigned char byte2 = ReadByte( TileDataAddress + line * 2 + 1 );

            // 
            for(int pixel = 0; pixel < 8; pixel++)
            {
                
                int PixelOffset = pixel * 3;

                unsigned char bit1 = (byte1 & (0x80 >> pixel));
                unsigned char bit2 = (byte2 & (0x80 >> pixel));

                unsigned char *colors;
                colors = GetPixelColor(bit1, bit2, 2);

                SpriteExplorerDisplay[tileOffset + lineoffset + PixelOffset] = colors[0];
                SpriteExplorerDisplay[tileOffset + lineoffset + PixelOffset + 1] = colors[1];
                SpriteExplorerDisplay[tileOffset + lineoffset + PixelOffset + 2] = colors[2];

                // DisplayPixels[tileOffset + lineoffset + PixelOffset] = colors[0];
                // DisplayPixels[tileOffset + lineoffset + PixelOffset + 1] = colors[1];
                // DisplayPixels[tileOffset + lineoffset + PixelOffset + 2] = colors[2];
    
                
            }
        }
    }


}

void LoadLineFromMap (void)
{
    
    if(lcd.LY == 0) 
    {
        WindowXFlag = 0; //reset window criteria every frame
        WindowYFlag = 0;
        WindowLineCounter = 0;
    }

    unsigned short DataAddress = GetDataAddress();
    unsigned short MapAddress;

    int TileNum;
    int TileID;
    int offset;

    unsigned char byte1;
    unsigned char byte2;
    unsigned char bit1;
    unsigned char bit2;

    unsigned char Ypos = lcd.LY + lcd.SCY;

    if(lcd.WX < 7) WindowXFlag = 1;
    if(lcd.LY == lcd.WY) WindowYFlag = 1; 

    for (int Pixelcounter = 0; Pixelcounter < 160; Pixelcounter++)
    {
        unsigned char Xpos = lcd.SCX + Pixelcounter;
        
        int currentBGTile = ((Ypos)/8)*32 + ((Xpos)/8); //The tile to be writing in this position.
        if(Pixelcounter == lcd.WX - 7) WindowXFlag = 1;
        
        if((Xpos) % 8 == 0 || Pixelcounter == 0) //when to grab new tile
        {
            if(UsingWindow() && WindowYFlag && WindowXFlag) 
            {
                MapAddress = GetWINMAPAddress();
                TileNum = (WindowLineCounter/8) * 32 + (Pixelcounter/8);
                TileID = (DataAddress == 0x8000) ? (unsigned char) ReadByte(MapAddress + TileNum) : (signed char) ReadByte(MapAddress + TileNum);
                offset = (DataAddress == 0x8000) ? (TileID * 16) : ((TileID + 128) * 16); 
                
                byte1 = ReadByte(DataAddress + offset + (WindowLineCounter % 8) * 2);
                byte2 = ReadByte(DataAddress + offset + (WindowLineCounter % 8) * 2 + 1);
            } 
            else 
            {
                MapAddress = GetBGMAPAddress();
                TileID = (DataAddress == 0x8000) ? (unsigned char) ReadByte(MapAddress + currentBGTile) : (signed char) ReadByte(MapAddress + currentBGTile);
                offset = (DataAddress == 0x8000) ? (TileID * 16) : ((TileID + 128) * 16); 

                byte1 = ReadByte(DataAddress + offset + (Ypos % 8) * 2);
                byte2 = ReadByte(DataAddress + offset + (Ypos % 8) * 2 + 1);
            }
        }

        unsigned char bit1 = (byte1 & (0x80 >> (Xpos % 8)));
        unsigned char bit2 = (byte2 & (0x80 >> (Xpos % 8)));

        unsigned char *colors;
        colors = GetPixelColor(bit1, bit2, 2);
        if(!(lcd.control & 0x01)) colors = GetPixelColor(0, 0, 2);

        DisplayPixels[lcd.LY * 160 * 3 + Pixelcounter * 3] = colors[0];
        DisplayPixels[lcd.LY * 160 * 3 + Pixelcounter * 3 + 1] = colors[1];
        DisplayPixels[lcd.LY * 160 * 3 + Pixelcounter * 3 + 2] = colors[2];

    }

    if(UsingWindow() && WindowYFlag && WindowXFlag) WindowLineCounter++;
}

unsigned char * GetPixelColor (unsigned char bit1, unsigned char bit2, int pallette)
{

    struct Pallette {

        union{
            
            unsigned char data;
            struct {

            unsigned char index0 : 2;
            unsigned char index1 : 2;
            unsigned char index2 : 2;
            unsigned char index3 : 2;

            };
        };
        

    };

    struct Pallette colorway;
    switch (pallette)
    {
    case 2: //BG
        colorway.data = ReadByte(0xFF47);
        break;
    case 1:
        colorway.data = ReadByte(0xFF49);
        break;
    case 0:
        colorway.data = ReadByte(0xFF48);
        break;
    default:
        break;
    }

    static unsigned char Colors[12] = {

        //white, light, dark, black
        0xE0, 0xF8, 0xD0,
        0x88, 0xC0, 0x70,
        0x34, 0x68, 0x56,
        0x08, 0x18, 0x20

    };

    //{0xE0, 0xF8, 0xD0}
    if (bit1 && bit2) 
    {
        //index 3
        
        return &Colors[colorway.index3 * 3]; 
    } else if (bit2) {

        //index 2
        return &Colors[colorway.index2 * 3];  

    } else if (bit1) {

        //index 1
        return &Colors[colorway.index1 * 3];  

    } else {

        //index 0
        return &Colors[colorway.index0 * 3];  

    } 

    
}
    