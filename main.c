#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdbool.h>
#include <windows.h>

#include "header.h"
#include "memory.h"
#include "control.h"
#include "display.h"
#include "gpu.h"

int WindowWidth = 160 * 5;
int WindowHeight = 144 * 5;

void Test (void) 
{
    registers.AF = 0x0000;
    registers.BC = 0x1200;
    registers.DE = 0x0000;
    registers.HL = 0xC304;
    registers.SP = 0xDFFD;
    registers.PC = 0xC33D;

    DAA();

    printf("A: %02X F: %02X B: %02X C: %02X D: %02X E: %02X H: %02X L: %02X SP: %04X PC: 00:%04X (%02X %02X %02X %02X)\n", 
                            registers.A, registers.F, registers.B, registers.C, registers.D, registers.E, registers.H, registers.L, registers.SP, registers.PC, 
                            ReadByte(registers.PC), ReadByte(registers.PC + 1), ReadByte(registers.PC + 2), ReadByte(registers.PC + 3));

    exit(0);
}

void DrawViewport (SDL_Renderer* renderer, SDL_Texture* texture)
{
    int W = WindowWidth;
    int H = WindowHeight;

    SDL_RenderClear(renderer);
    SDL_UpdateTexture(texture, NULL, LoadTilesFromMap(0x9800, 0x8000), 256*3);
    
    if((lcd.SCX + 144) >= 256 && (lcd.SCY + 160) >= 256) //scrolling bothways
    {

        //to implement

    }
    else if ((lcd.SCX + 144) >= 256) //scrolling sideways only
    {
        SDL_Rect FirstScrollsrcrect = { //current display at SCX: 0 and SCY: 0.
            lcd.SCX, lcd.SCY, (256 - lcd.SCX), 8 * 18
        };

        SDL_Rect FirstScrolldstrect = { //current display at SCX: 0 and SCY: 0.
            0, 0, ((256 - lcd.SCX)/256.0) * W, H
        };

        SDL_Rect SecondScrollsrcrect = { //current display at SCX: 0 and SCY: 0.
            0, lcd.SCY, ((lcd.SCX + 144) - 256 ), 8 * 18
        };

        SDL_Rect SecondScrolldstrect = { //current display at SCX: 0 and SCY: 0.
            ((256-lcd.SCX)/256.0) * W, 0, (lcd.SCX) /256.0 * W, H
        };

        SDL_RenderCopy(renderer, texture, &FirstScrollsrcrect, &FirstScrolldstrect);
        SDL_RenderCopy(renderer, texture, &SecondScrollsrcrect, &SecondScrolldstrect);
        SDL_RenderPresent(renderer);
    }
    else if ((lcd.SCY + 160) >= 256) //scrolling downwards only
    {
        SDL_Rect FirstScrollsrcrect = { //current display at SCX: 0 and SCY: 0.
            lcd.SCX, lcd.SCY, 8 * 20, (256 - lcd.SCY)
        };

        SDL_Rect FirstScrolldstrect = { //current display at SCX: 0 and SCY: 0.
            0, 0, W, ((256 - lcd.SCY)/256.0) * H
        };

        SDL_Rect SecondScrollsrcrect = { //current display at SCX: 0 and SCY: 0.
            lcd.SCX, 0, 8 * 20, ((lcd.SCY + 144) - 256 )
        };

        SDL_Rect SecondScrolldstrect = { //current display at SCX: 0 and SCY: 0.
            0, ((256-lcd.SCY)/256.0) * H, W, ((lcd.SCY) /256.0) * H
        };

        SDL_RenderCopy(renderer, texture, &FirstScrollsrcrect, &FirstScrolldstrect);
        SDL_RenderCopy(renderer, texture, &SecondScrollsrcrect, &SecondScrolldstrect);
        SDL_RenderPresent(renderer);

    } 
    else //no scrolling
    {
        SDL_Rect srcrect = { //current display at SCX: 0 and SCY: 0.
            lcd.SCX, lcd.SCY, 8 * 20, 8 * 18
        };
        
        SDL_RenderCopy(renderer, texture, &srcrect, NULL);
        SDL_RenderPresent(renderer);
    }


}

// void PushDrawEvent (void) 
// {
//     SDL_PushEvent() 
// }

int main(int argc, char** argv){

    LoadRom();
    Reset();
    remove("Log.txt");
    // Test();

    if(SDL_Init(SDL_INIT_VIDEO) < 0){
        printf("Error: SDL failed to initialize\nSDL Error: '%s'\n", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("Fatboy", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
                                            WindowWidth, WindowHeight, SDL_WINDOW_RESIZABLE);
    
    if(!window){
        printf("Error: Failed to open window\nSDL Error: '%s'\n", SDL_GetError());
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if(!renderer){
        printf("Error: Failed to create renderer\nSDL Error: '%s'\n", SDL_GetError());
        return 1;
    }

    
    SDL_Texture * texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, 
                                            SDL_TEXTUREACCESS_STREAMING, 256, 256);

    unsigned char NfromLog[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                0xF0, 0x00, 0xF0, 0x00, 0xFC, 0x00, 0xFC, 0x00, 
                                0xFC, 0x00, 0xFC, 0x00, 0xF3, 0x00, 0xF3, 0x00};
    
    
    memcpy(VRAM, NfromLog, 32);
    //WriteByte(0x9800, 0x01);

    

    bool running = true;
    while(running){


        SDL_Event event;
        while(SDL_PollEvent(&event)){

            switch(event.type){
                
                
                case SDL_QUIT:

                    SDL_DestroyTexture(texture);
                    SDL_DestroyRenderer(renderer);
                    SDL_DestroyWindow(window);
                    
                    running = false;
                    break;

                default:
                    break;
            }
            
        }
        
        
        DrawViewport(renderer, texture);
        Update(); //assuming this takes and insignificant amount of time
        // Sleep(1000/60);
        
        
        // LoadTilesFromMap(0x9800, 0x8000)
    }

    return 0;
    
}