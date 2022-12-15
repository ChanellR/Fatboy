#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdbool.h>     
 
int main(int argc, char ** argv)
{
    bool quit = false;
    SDL_Event event;
 
    SDL_Init(SDL_INIT_VIDEO);
 
    SDL_Window * window = SDL_CreateWindow("SDL2 Displaying Image",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, 0);
 

    unsigned char pixels[24] = {0,0,0,
                                000, 150, 150,
                                0,0,0,
                                0,0,0,
                                0,0,0,
                                0,0,0,
                                0,0,0,
                                0,0,0
                                 };

    SDL_Renderer * renderer = SDL_CreateRenderer(window, -1, 0);
    SDL_Surface * image = SDL_CreateRGBSurfaceWithFormatFrom(pixels, 4, 2, 8, 
                                                        3 * 4, SDL_PIXELFORMAT_RGB24);

    // SDL_Surface * image = SDL_LoadBMP("image.bmp");
    if(!image){
        printf("Error: Failed to open image\nSDL Error: '%s'\n", SDL_GetError());
        return 1;
    }
    // SDL_Texture * texture = SDL_CreateTextureFromSurface(renderer, image);
    SDL_Texture* texture = SDL_CreateTexture(renderer, 
                            SDL_PIXELFORMAT_RGB24,
                            SDL_TEXTUREACCESS_TARGET,
                            4, 
                            2
                            );


    SDL_UpdateTexture(texture, 
                NULL,
                pixels, 
                4 * 3);


    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);    

    while (!quit)
    {
        SDL_WaitEvent(&event);
 
        switch (event.type)
        {
        case SDL_QUIT:
            SDL_DestroyTexture(texture);
            SDL_FreeSurface(image);
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(window);
            quit = true;
            break;
        }
    }
 
    // SDL_Quit();
 
    return 0;
}