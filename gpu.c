#include <stdio.h>
#include <string.h>
#include <windows.h>

#include "header.h"
#include "memory.h"
#include "control.h"
#include "display.h"
#include "gpu.h"

 //every dot and their hue

struct LCD lcd;
int ScanlineCounter = 456; //456 cycles per scan line, there is a 146 vblank buffer
unsigned char DisplayPixels [256 * 256 * 3];
int printLine = -10000000;

unsigned char* GetPixelColor (unsigned char bit1, unsigned char bit2);

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
        ScanlineCounter -= (cycles * 4);
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
            
            lcd.LY = 0; //reset if outside range
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

    } else if (ScanlineCounter < 456){ //to buffer in the beginning 

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


        if (ReqInt && (currentMode != mode)) {
            RequestInterrupt(1); //stat interrupt
        }

        if (lcd.LY == lcd.LYC) {
            lcd.status |= 0x04;

            if(lcd.status & 0x40) { //bit 6
                RequestInterrupt(1);
            }

        } else {
            lcd.status &= (0xFB); //everything but second bit
        }


    }

}

void DrawScanline(void) {

    if(lcd.control & 0x80) {
        
        LoadLineFromMap();

    } 
    
    if (lcd.control & 0x02) {
        RenderSprites();
    }
}

void RenderSprites (void) {

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

void LoadSpriteSheet (void)
{   
    //displays all tiles currently mapped in the BG, doesn't hold scanline functionality but makes sense. 
    //returns 8 x 8 pixel bitmap
    //unsigned only 0x8000-

    unsigned short DataAddress = 0x8000;
    
    for (int tile = 0; tile < ( 32 * 12 ); tile++)
    {
        int tileOffset = (tile % 16) * 8 * 3 + ((tile / 16) * 256 * 3 * 8); //row then coloumn
         
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
                colors = GetPixelColor(bit1, bit2);

                DisplayPixels[tileOffset + lineoffset + PixelOffset] = GetPixelColor(bit1, bit2)[0];
                DisplayPixels[tileOffset + lineoffset + PixelOffset + 1] = GetPixelColor(bit1, bit2)[1];
                DisplayPixels[tileOffset + lineoffset + PixelOffset + 2] = GetPixelColor(bit1, bit2)[2];
                
            }
        }

        DataAddress += 16; //each tile move forward 16 bytes
    }


}

void LoadTilesFromMap (void)
{   
    if(!strcmp(Title, "DR.MARIO")) {lcd.SCY = 0;}
    //displays all tiles currently mapped in the BG, doesn't hold scanline functionality but makes sense. 
    //returns 8 x 8 pixel bitmap
    //unsigned only 0x8000-
    unsigned short MapAddress = (lcd.control) ? GetBGMAPAddress() : 0x9800; 
    unsigned short DataAddress = (lcd.control) ? GetDataAddress() : 0x8000;
    
    unsigned short TileIDAddress = MapAddress;
    //BG
    
    for (int tile = 0; tile < ( 32 * 32 ); tile++)
    {
        
        int tileOffset = (tile % 32) * 8 * 3 + ((tile / 32) * 256 * 3 * 8); //row then coloumn

        unsigned char TileID = (DataAddress == 0x8000) ? (unsigned char) ReadByte(TileIDAddress + tile) : (signed char) ReadByte(TileIDAddress + tile);
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
                colors = GetPixelColor(bit1, bit2);

                DisplayPixels[tileOffset + lineoffset + PixelOffset] = GetPixelColor(bit1, bit2)[0];
                DisplayPixels[tileOffset + lineoffset + PixelOffset + 1] = GetPixelColor(bit1, bit2)[1];
                DisplayPixels[tileOffset + lineoffset + PixelOffset + 2] = GetPixelColor(bit1, bit2)[2];
                
            }
        }
    }


}

void LoadLineFromMap (void)
{
    //scrolls automatically to fit everything within 160 * 144 in display
    //saves from having to dynamically asdjust the src and dst rect which would case distortion
    
    unsigned short DataAddress = (lcd.control) ? GetDataAddress() : 0x8000;
    unsigned short TileIDAddress = (lcd.control) ? GetBGMAPAddress() : 0x9800;

    //find the first tile in the line and the last one
    //num of first tile in line with SCX
    
    if(!strcmp(Title, "DR.MARIO")) lcd.SCY = 0;
    else if(!strcmp(Title, "DMG-ACID2")) lcd.SCY = 0; //unexpected behavior with SCY

    int Tileshift = (lcd.SCX/8);

    int firstTileInLine = ((((unsigned char)lcd.LY + lcd.SCY) / 8) * 32);    
    int lastTileInLine = (((unsigned char)(lcd.LY + lcd.SCY )/ 8) * 32) + 31; // char so it scrolls

    int firstTiletoWrite = (((unsigned char)(lcd.LY + lcd.SCY) / 8) * 32) + Tileshift;
    
    for (int tile = firstTiletoWrite; tile < firstTiletoWrite + 20; tile++) //dont do 32 tiles after
    {

        tile = (tile > lastTileInLine) ? firstTileInLine + (tile - lastTileInLine) : tile;
        
        int tileOffset = ((tile - firstTiletoWrite) % 32) * 8 * 3; //row then coloumn

        //if using window, check tile
        int WindowStartingTile = ( (lcd.WX - 7) / 8);

        if(UsingWindow() && (lcd.LY >= lcd.WY) && ((tile % 32) >= WindowStartingTile)) TileIDAddress = (lcd.control) ? GetWINMAPAddress() : 0x9800;
        
        unsigned char TileID = (DataAddress == 0x8000) ? (unsigned char) ReadByte(TileIDAddress + tile) : (signed char) ReadByte(TileIDAddress + tile);
        int offset = (DataAddress == 0x8000) ? (TileID * 16) : ((TileID + 128) * 16);

        unsigned short TileDataAddress = DataAddress;
        TileDataAddress += (offset); 

        int line = (lcd.LY + lcd.SCY) % 8;
        int lineoffset = (lcd.LY) * (256) * 3;// should start printing only @lcd.LY independent of positon of lcd.SCY

        unsigned char byte1 = ReadByte( TileDataAddress + line * 2); //off set by their position in the tile
        unsigned char byte2 = ReadByte( TileDataAddress + line * 2 + 1 );

        for(int pixel = 0; pixel < 8; pixel++)
        {
            
            int PixelOffset = pixel * 3;

            unsigned char bit1 = (byte1 & (0x80 >> pixel));
            unsigned char bit2 = (byte2 & (0x80 >> pixel));

            unsigned char *colors;
            colors = GetPixelColor(bit1, bit2);

            DisplayPixels[tileOffset + lineoffset + PixelOffset] = GetPixelColor(bit1, bit2)[0];
            DisplayPixels[tileOffset + lineoffset + PixelOffset + 1] = GetPixelColor(bit1, bit2)[1];
            DisplayPixels[tileOffset + lineoffset + PixelOffset + 2] = GetPixelColor(bit1, bit2)[2];
            
        }
        
    }
}


unsigned char * GetPixelColor (unsigned char bit1, unsigned char bit2)
{

    if (bit1 && bit2) 
    {
        //black
        // return (unsigned char []){0x08, 0x18, 0x20};
        return (unsigned char []){0x08, 0x18, 0x20};
    } else if (bit1) {

        //light grey
        return (unsigned char []){0x88, 0xC0, 0x70};

    } else if (bit2) {

        //dark grey
        return (unsigned char []){0x34, 0x68, 0x56};

    } else {

        //white
        return (unsigned char []){0xE0, 0xF8, 0xD0};

    } 
    
}
    