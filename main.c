#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdbool.h>
#include <windows.h>

#include "header.h"
#include "memory.h"
#include "control.h"
#include "display.h"
#include "gpu.h"

#define SCREEN_WIDTH 900 
#define SCREEN_HEIGHT 900

void DisplayGraphics (unsigned char ** stream, SDL_Renderer * render);
void CreatePixelStream (unsigned short Address);
unsigned char Stream[64 * 2];


int main(int argc, char** argv){

    if(SDL_Init(SDL_INIT_VIDEO) < 0){
        printf("Error: SDL failed to initialize\nSDL Error: '%s'\n", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("SLD test", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    
    if(!window){
        printf("Error: Failed to open window\nSDL Error: '%s'\n", SDL_GetError());
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if(!renderer){
        printf("Error: Failed to create renderer\nSDL Error: '%s'\n", SDL_GetError());
        return 1;
    }


    LoadRom();
    Reset();
    remove("Log.txt");

    bool running = true;
    while(running){

        // SDL_Event event;
        // while(SDL_PollEvent(&event)){

        //     switch(event.type){
        //         case SDL_QUIT:
        //             running = false;
        //             break;

        //         default:
        //             break;
        //     }

        // }

        Update(); //assuming this takes and insignificant amount of time
        // DisplayGraphics(&WindowData, renderer);
        // Sleep(1000/60); //update 60 times a second

        
        // CreatePixelStream(0x8010);
        // DrawTile(&Stream, renderer, 0, 0);
        
        // SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        // SDL_RenderPresent(renderer);
                
    }

    return 0;
    
}

/*
Read the first 16bytes 
reconstruct a sprite image 
unsigned char tile[16][]
*/

void CreatePixelStream (unsigned short Address) {


    for (int i = 0; i < 8; i++)
    {

        unsigned char byte1 = ReadByte(Address + 2*i);
        unsigned char byte2 = ReadByte(Address + 2*i + 1);

        for (int j = 0; j < 8; j++)
        {
            int bit1 = (byte1 & (1 << (7-j)));
            int bit2 = (byte2 & (1 << (7-j)));

            int colorVal = 3;
            
            if (bit1 && bit2) 
            {

                colorVal = 0;

            } else if (bit2) {

                colorVal = 2;

            } else if (bit1) {

                colorVal = 1;

            } 

            Stream[i*8 + j] = (unsigned char) colorVal;   
        }

    }

    
}

void DisplayGraphics (unsigned char ** stream, SDL_Renderer * render)
{
    int DotWidth = 160;
    int DotHeight = 144;
    int Pixel_width = SCREEN_WIDTH / DotWidth; //160 x 144 Pixels are rectangles
    int Pixel_height = SCREEN_HEIGHT / DotHeight;

    SDL_Rect * Pixels = malloc((DotWidth * DotHeight) * sizeof(SDL_Rect));

    for (int i = 0; i < DotHeight; i++) //y 144
    {
        for (int j = 0; j < DotWidth; j++) //x 160
        {

            Pixels[i*160 + j].x = j * Pixel_width;
            Pixels[i*160 + j].y = i * Pixel_height;

            Pixels[i*160 + j].h = Pixel_height;
            Pixels[i*160 + j].w = Pixel_width;
            
        }

    }

    for (int k = 0; k < (DotHeight); k++) {

        for (int h = 0; h < (DotWidth); h++){

            unsigned char r;
            unsigned char g;
            unsigned char b;


            switch(stream[k][h]){
                case 0:

                    r = g = b = 0;
                    break;

                case 1:

                    r = g = b = 0x77;
                    break;

                case 2:

                    r = g = b = 0xCC;
                    break;

                case 3:

                    r = g = b = 255;
                    break;

            }

            SDL_SetRenderDrawColor(render, r, g, b, 255);
            SDL_RenderFillRect(render, &Pixels[k*160 + h]);
        }
    }
}


