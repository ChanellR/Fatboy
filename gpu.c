#include <stdio.h>

#include "header.h"
#include "memory.h"
#include "control.h"
#include "display.h"
#include "gpu.h"

 //every dot and their hue

struct LCD lcd;
int ScanlineCounter = 456 + 35; //456 cycles per scan line, there is a 146 vblank buffer
unsigned char WindowData [144][160];

void UpdateGraphics (int cycles) {

    SetLCDstatus();

    if(IsLcdOn()) {
        ScanlineCounter -= cycles;
    } else {
        return;
    }
    
    if (ScanlineCounter <= 0) {

        lcd.LY++;
        ScanlineCounter = 456;



        if(lcd.LY == 144) {

            RequestInterrupt(0);

        } else if (lcd.LY > 153) {
            
            lcd.LY = 0; //reset if outside range

        } else if (lcd.LY < 144) {
            
            // DrawScanline(); //this will add a line to the render buffer
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
            printf("requested");
            RequestInterrupt(1); //stat interrupt
        }

        if (lcd.LY == lcd.LYC) {
            lcd.status |= 0x04;

            if(lcd.status & 0x40) { //bit 6
                CreateBox("LYC interrupt.");
                RequestInterrupt(1);
            }

        } else {
            lcd.status &= (0xFB); //everything but second bit
        }


    }

}

void DrawScanline(void) {

    if(lcd.control & 0x01) {
        RenderTiles();
    } 
    
    if (lcd.control & 0x02) {
        RenderSprites();
    }
}

void RenderTiles(void) {

    // first figure out where the layout is stored
    unsigned short TileMapAddress;
    unsigned short TileDataAddress;
    unsigned char WindowEnabled = 0;
    unsigned char unsig = 1;

    //BG & Window Data
    if (lcd.control & 0x10) {

        TileDataAddress = 0x8000;

    } else {
    
        TileDataAddress = 0x8800;
        unsig = 0; //signed data Address.

    }

    if(lcd.control & 0x20) {

        if (lcd.LY >= lcd.WY) WindowEnabled = 1; //if in window space, above windows top left corner 

    }

    //Map areas

    
    if(WindowEnabled){

        if(lcd.control & 0x40) {

        TileMapAddress = 0x9C00;

        } else {

            TileMapAddress = 0x9800;
            
        }

    } else {

        if(lcd.control & 0x08) { //read from Background part for the address instead.

        TileMapAddress = 0x9C00;

        } else {

            TileMapAddress = 0x9800;

        }
    }

    //no to move onto actually drawing the line

    unsigned char Ypos = 0; //check if this should always displaced if not in window, scroll Y + is scaring me
    
    if(!WindowEnabled) {
        Ypos = lcd.SCY + lcd.LY;
    } else {
        Ypos = lcd.LY - lcd.WY; //move back until the beginning of the window. Does it wrap around?
    }

    unsigned char TileRow = (Ypos / 8); //integer divides down

    for (int Pixel = 0; Pixel < 160; Pixel++){ //Pixel because they can have uneven offsets

        unsigned char Xpos = Pixel + lcd.SCX;

        if(WindowEnabled) {

            if (Pixel >= lcd.WX) {
                
                Xpos = Pixel - lcd.WX;
                //no minus 7?
            }  

        }

        //find what tile is being read
        //The tiles map has id's in line with all 32 x 32 tiles in order

        unsigned char TileCol = (Xpos / 8);
        signed char TileID;

        //read the ID byte from the map to find the data for that tile

        unsigned short TileIDAddress = TileMapAddress + (TileRow * 32) + (TileCol);

        if(unsig){
            TileID = (unsigned char) ReadByte(TileIDAddress);
        } else {
            TileID = (signed char) ReadByte(TileIDAddress);
        }


        if (unsig) { //offsets the tiledata address to find the tile data
            TileDataAddress += (TileID * 16); 
        } else {
            TileDataAddress += ((TileID + 128) * 16);
        }

        unsigned char byte1 = ReadByte( TileDataAddress + (Ypos % 8) * 2 ); //off set by their position in the tile
        unsigned char byte2 = ReadByte( TileDataAddress + (Ypos % 8) * 2 + 1 );

        unsigned char TileBit = (Xpos % 8);
 
        int bit1 = (byte1 & (1 << (7-TileBit)));
        int bit2 = (byte2 & (1 << (7-TileBit)));

        int colorVal = 3;
        
        if (bit1 && bit2) 
        {
            colorVal = 0;

        } else if (bit2) {

            colorVal = 2;

        } else if (bit1) {

            colorVal = 1;

        } 

        WindowData[lcd.LY][Pixel] =  (unsigned char) colorVal; //input into the specific row and pixel

    }

}

void RenderSprites (void) {

}