#include <stdio.h>

#include "header.h"
#include "memory.h"
#include "control.h"
#include "display.h"
#include "gpu.h"

 //every dot and their hue

struct LCD lcd;
int ScanlineCounter = 456 + 35; //456 cycles per scan line, there is a 146 vblank buffer

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
            printf("requested");
            RequestInterrupt(1); //stat interrupt
        }

        if (lcd.LY == lcd.LYC) {
            lcd.status |= 0x04;

            if(lcd.status & 0x40) { //bit 6
                CreateBox("LYC interrupt.");
                RequestInterrupt(1);
            } else {}

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

