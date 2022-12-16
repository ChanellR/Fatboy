#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdbool.h>
#include <windows.h>

#include "header.h"
#include "memory.h"
#include "control.h"
#include "display.h"
#include "gpu.h"
#include "src\include\SDL2\SDL_syswm.h"

#define WM_MESSAGE(x) (x.syswm.msg->msg.win)

#define IDM_FILE_OPEN 1
#define IDM_FILE_QUIT 2
#define IDM_VIEW_SWAP 3
#define IDM_VIEW_SPRITE 4


char CurrentRom[40];

int WindowWidth = 160 * 5;
int WindowHeight = 144 * 5;
int RomLoaded = 0;

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

HWND getSDLWinHandle(SDL_Window* win)
{
    SDL_SysWMinfo infoWindow;
    SDL_VERSION(&infoWindow.version);
    if (!SDL_GetWindowWMInfo(win, &infoWindow))
    {
        printf("test");
        return NULL;
    }
    return (infoWindow.info.win.window);
}

HMENU hMenuView;

void CreateMenuBar (HWND hwnd)
{

    HMENU hMenubar;
    HMENU hMenuFile;
    

    hMenubar = CreateMenu();
    hMenuFile = CreateMenu();
    hMenuView = CreateMenu();


    AppendMenuW(hMenuFile, MF_STRING, IDM_FILE_OPEN, L"&Open");
    AppendMenuW(hMenuFile, MF_SEPARATOR, 0, NULL);
    AppendMenuW(hMenuFile, MF_STRING, IDM_FILE_QUIT, L"&Quit");

    AppendMenuW(hMenuView, MF_STRING, IDM_VIEW_SWAP, L"&ViewPort");
    CheckMenuItem(hMenuView, IDM_VIEW_SWAP, MF_UNCHECKED);
    AppendMenuW(hMenuView, MF_STRING, IDM_VIEW_SPRITE, L"&View Sprite Sheet");
    CheckMenuItem(hMenuView, IDM_VIEW_SWAP, MF_UNCHECKED);

    AppendMenuW(hMenubar, MF_POPUP, (UINT_PTR) hMenuFile, L"&File");
    AppendMenuW(hMenubar, MF_POPUP, (UINT_PTR) hMenuView, L"&View");

    SetMenu(hwnd, hMenubar);

}

void OpenDialog(HWND hwnd) {

  OPENFILENAME ofn;
  TCHAR szFile[MAX_PATH];

  ZeroMemory(&ofn, sizeof(ofn));
  ofn.lStructSize = sizeof(ofn);
  ofn.lpstrFile = szFile;
  ofn.lpstrFile[0] = '\0';
  ofn.hwndOwner = hwnd;
  ofn.nMaxFile = sizeof(szFile);
  ofn.lpstrFilter = TEXT("All files(*.*)\0*.*\0");
  ofn.nFilterIndex = 1;
  ofn.lpstrInitialDir = NULL;
  ofn.lpstrFileTitle = NULL;
  ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

  if(GetOpenFileNameA(&ofn))
  {
    Reset();  
    LoadRom(ofn.lpstrFile);
    LoadTilesFromMap();

  }

}



void DrawViewportWithScrolling (SDL_Renderer* renderer, SDL_Texture* texture, int map)
{
    //map: display entire 32 * 32 tile map, isn't drawn by default by LoadLineFromMap

    SDL_Rect srcrect = { //current display at SCX: 0 and SCY: 0.
            0, 0, 8 * 20, 8 * 18
    };

    SDL_RenderClear(renderer);
    SDL_UpdateTexture(texture, NULL, DisplayPixels, 256*3); //changed for logo
    
    SDL_RenderCopy(renderer, texture, 
                    (map) ? NULL : &srcrect, 
                    NULL); //just display entire field for now

    SDL_RenderPresent(renderer);

    return;

}


int main(int argc, char** argv){

    int fps = 60;
    int DesiredDelta = 1000 / fps;
    int Viewport = 1; //0: viewport, 1: map, 2: sprite sheet
    int ViewSpriteSheet = 0;
    
    Reset();

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

    HWND WindowHandler = getSDLWinHandle(window);
      
    
    SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);
    CreateMenuBar(WindowHandler); 
    
    
    bool running = true;
    while(running){

        int startLoop = SDL_GetTicks();

        SDL_Event event;
        while(SDL_PollEvent(&event)){

            switch(event.type){

                case SDL_SYSWMEVENT:

                    switch(WM_MESSAGE(event).msg){

                        case WM_COMMAND:

                            switch (LOWORD(WM_MESSAGE(event).wParam))
                            {

                                case IDM_FILE_OPEN:

                                    MessageBeep(MB_ICONINFORMATION);
                                    OpenDialog(WindowHandler);
                                    break;
                                    
                                
                    
                                case IDM_FILE_QUIT:
                                
                                    RomLoaded = 0;
                                    SendMessage(WindowHandler, WM_CLOSE, 0, 0);
                                    break; 

                                case IDM_VIEW_SWAP:
                                
                                    UINT state = GetMenuState(hMenuView, IDM_VIEW_SWAP, MF_BYCOMMAND); 

                                    if (state == MF_CHECKED) {

                                        Viewport = 1;
                                        CheckMenuItem(hMenuView, IDM_VIEW_SWAP, MF_UNCHECKED);  

                                    } else {

                                        Viewport = 0;
                                        CheckMenuItem(hMenuView, IDM_VIEW_SWAP, MF_CHECKED);  
                                    }
                    
                                    break;

                                case IDM_VIEW_SPRITE:

                                    state = GetMenuState(hMenuView, IDM_VIEW_SPRITE, MF_BYCOMMAND); 

                                    if (state == MF_CHECKED) {

                                        ViewSpriteSheet = 0;
                                        CheckMenuItem(hMenuView, IDM_VIEW_SPRITE, MF_UNCHECKED);  

                                    } else {

                                        ViewSpriteSheet = 1;
                                        Viewport = 1;
                                        CheckMenuItem(hMenuView, IDM_VIEW_SWAP, MF_UNCHECKED); //zoom out for sprites
                                        CheckMenuItem(hMenuView, IDM_VIEW_SPRITE, MF_CHECKED);  

                                    }
                            }       break;


                        break;

                        case WM_KEYDOWN:
                            
                            switch (LOWORD(WM_MESSAGE(event).wParam))
                            {
                                case VK_LEFT:

                                    
                                    joypad.keys &= ~0x22;
                                    RequestInterrupt(4);
                                    break;

                                case VK_RIGHT:

                                    
                                    joypad.keys &= ~0x21;
                                    RequestInterrupt(4);
                                    break;

                                case VK_UP:

                                    
                                    joypad.keys &= ~0x24;
                                    RequestInterrupt(4);
                                    break;

                                case VK_DOWN:

                                    
                                    joypad.keys &= ~0x28;
                                    RequestInterrupt(4);
                                    break;
                                
                                case VK_RETURN:
                                    //start
                                    
                                    joypad.keys &= ~0x18;
                                    RequestInterrupt(4);
                                    break;

                                case VK_SHIFT:
                                    //select
                                    
                                    joypad.keys &= ~0x14;
                                    RequestInterrupt(4);
                                    break;

                                case 0x5A: //Z (A)

                                    
                                    joypad.keys &= ~0x11;
                                    RequestInterrupt(4);
                                    break;

                                case 0x58: //X (B)

                                    
                                    joypad.keys &= ~0x12;
                                    RequestInterrupt(4);
                                    break;
                            }

                        break;
                    }

                    break;

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
        
        
        Update(); //assuming this takes an insignificant amount of time

        //  
        if(ViewSpriteSheet) {memset(DisplayPixels, 0, 256 * 256 * 3); LoadSpriteSheet();}
        LoadTilesFromMap();
        DrawViewportWithScrolling(renderer, texture, Viewport);
        

        int delta = SDL_GetTicks() - startLoop;
        if(delta < DesiredDelta) SDL_Delay(DesiredDelta - delta);


    }

    return 0;
    
}