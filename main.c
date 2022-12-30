#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdbool.h>
#include <windows.h>

#include "header.h"
#include "memory.h"
#include "control.h"
#include "ppu.h"

#include "src\include\SDL2\SDL_syswm.h"

#define WM_MESSAGE(x) (x.syswm.msg->msg.win)

#define IDM_FILE_OPEN 1
#define IDM_FILE_QUIT 2
#define IDM_VIEW_MAP 3
#define IDM_VIEW_SHEET 4
#define IDM_VIEW_SPRITES 5
#define IDM_FILE_SAVE 6
#define IDM_FILE_LOAD 7

char CurrentRom[40];

int WindowWidth = 160 * 5;
int WindowHeight = 144 * 5;
int RomLoaded = 0;
int triggered = 0;

HMENU hMenuView;

void Test (void) 
{
    registers.AF = 0x0A11;
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
        printf("No Window Handler!");
        return NULL;
    }
    return (infoWindow.info.win.window);
}

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
    AppendMenuW(hMenuFile, MF_STRING, IDM_FILE_SAVE, L"&Save");
    AppendMenuW(hMenuFile, MF_STRING, IDM_FILE_LOAD, L"&Load");

    AppendMenuW(hMenuView, MF_STRING, IDM_VIEW_SHEET, L"&View Sprite Sheet");
    AppendMenuW(hMenuView, MF_STRING, IDM_VIEW_MAP, L"&View Tile Map");
    AppendMenuW(hMenuView, MF_STRING, IDM_VIEW_SPRITES, L"&Open Explorer/View Sprites");

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
    remove("instructionLog.txt");
    LoadRom(ofn.lpstrFile);
    DetectMBC();
    //LoadTilesFromMap();

  }

}

void DrawViewport (SDL_Renderer* renderer, SDL_Texture* texture)
{
    SDL_RenderClear(renderer);
    SDL_UpdateTexture(texture, NULL, DisplayPixels, 160*3); //changed for logo
    
    SDL_RenderCopy(renderer, texture, 
                    NULL, 
                    NULL); //just display entire field for now

    SDL_RenderPresent(renderer);

    return;

}

int main(int argc, char** argv){

    int fps = 60;
    int DesiredDelta = 1000 / fps;
    int Viewport = 0; //0: viewport, 1: map, 2: sprite sheet
    int ViewSpriteSheet = 1;
    
    Reset();


    if(SDL_Init(SDL_INIT_VIDEO) < 0){
        printf("Error: SDL failed to initialize\nSDL Error: '%s'\n", SDL_GetError());
        return 1;
    }

    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
		printf("SDL failed to initialize : %s\n", SDL_GetError());
		
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
                                            SDL_TEXTUREACCESS_STREAMING, 160, 144);

    
    HWND WindowHandler = getSDLWinHandle(window);
    SDL_Window *SpriteExplorer;

    HWND SpriteExplorerHandler;

    SDL_Texture * SpriteExplorerTexture;
    SDL_Renderer *SpriteExplorerrenderer;

    CreateMenuBar(WindowHandler);  
    
    SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);
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

                                case IDM_FILE_SAVE:
                                    
                                    Save();
                                    break;

                                case IDM_FILE_LOAD:
                                    //Reset();
                                    Load();
                                    break;

                                case IDM_FILE_QUIT:
                                
                                    RomLoaded = 0;
                                    SendMessage(WindowHandler, WM_CLOSE, 0, 0);
                                    SendMessage(SpriteExplorerHandler, WM_CLOSE, 0, 0);
                                    break; 

                                case IDM_VIEW_SHEET:
                                    ViewSpriteSheet = 2;
                                    break;

                                case IDM_VIEW_MAP:

                                    ViewSpriteSheet = 1;
                                    break;
                                
                                case IDM_VIEW_SPRITES:
                                    ViewSpriteSheet = 0;
                                    if(triggered == 1) break;

                                    SpriteExplorer = SDL_CreateWindow("Explorer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
                                            600, 600, SDL_WINDOW_RESIZABLE);
                                    
                                    SpriteExplorerHandler = getSDLWinHandle(SpriteExplorer);
                                    SpriteExplorerrenderer = SDL_CreateRenderer(SpriteExplorer, -1, 0);

                                    SpriteExplorerTexture = SDL_CreateTexture(SpriteExplorerrenderer, 
                                                        SDL_PIXELFORMAT_RGB24, 
                                                        SDL_TEXTUREACCESS_STREAMING, 256, 256);
                                    
                                    triggered = 1;

                                    break;
                                    
                                
                            }       

                        break;

                        case WM_KEYDOWN:
                            
                            switch (LOWORD(WM_MESSAGE(event).wParam))
                            {
                                case VK_LEFT:

                                    JoypadState |= 0x02;
                                    RequestInterrupt(4);
                                    break;

                                case VK_RIGHT:

                                    JoypadState |= 0x01;
                                    RequestInterrupt(4);
                                    break;

                                case VK_UP:

                                    
                                    JoypadState |= 0x04;
                                    RequestInterrupt(4);
                                    break;

                                case VK_DOWN:

                                    
                                    JoypadState |= 0x08;
                                    RequestInterrupt(4);
                                    break;
                                
                                case VK_RETURN:
                                    //start
                                    
                                    JoypadState |= 0x80;
                                    RequestInterrupt(4);
                                    break;

                                case VK_SHIFT:
                                    //select
                                    
                                    JoypadState |= 0x40;
                                    RequestInterrupt(4);
                                    break;

                                case 0x5A: //Z (A)

                                    
                                    JoypadState |= 0x20;
                                    RequestInterrupt(4);
                                    break;

                                case 0x58: //X (B)

                                    
                                    JoypadState |= 0x10;
                                    RequestInterrupt(4);
                                    break;
                            }
                        
                        break;

                        case WM_KEYUP:
                            
                            switch (LOWORD(WM_MESSAGE(event).wParam))
                            {
                                case VK_LEFT:

                                    JoypadState &= ~(0x02);

                                    break;

                                case VK_RIGHT:

                                    JoypadState &= ~(0x01);

                                    break;

                                case VK_UP:

                                    JoypadState &= ~(0x04);

                                    break;

                                case VK_DOWN:

                                    JoypadState &= ~(0x08);

                                    break;
                                
                                case VK_RETURN:
                                    //start
                                    
                                    JoypadState &= ~(0x80);

                                    break;

                                case VK_SHIFT:
                                    //select
                                    
                                    JoypadState &= ~(0x40);

                                    break;

                                case 0x5A: //Z (A)

                                    JoypadState &= ~(0x20);

                                    break;

                                case 0x58: //X (B)                                   
                                    JoypadState &= ~(0x10);
                                    break;

                                case VK_OEM_PLUS:
                                    fps += 10;
                                    printf("frame limit: %d\n", fps);
                                    break;
                                case VK_OEM_MINUS:
                                    fps -= 10;
                                    fps = (fps == 0) ? 10 : fps;
                                    printf("frame limit: %d\n", fps);
                                    break;
                            }
                            
                        break;
                    }

                    break;

                case SDL_WINDOWEVENT:

                    if(!(event.window.event == SDL_WINDOWEVENT_CLOSE)) break;
                    if(SDL_GetWindowID(SpriteExplorer) == event.window.windowID)
                    {
                        SDL_DestroyWindow(SpriteExplorer);
                        SDL_DestroyTexture(SpriteExplorerTexture);
                        SDL_DestroyRenderer(SpriteExplorerrenderer);
                        triggered = 0;
                        break;
                    }
                
                case SDL_QUIT: //won't work with multiple windows
                    
                    SDL_DestroyRenderer(renderer);

                    SDL_DestroyTexture(texture);
                    SDL_DestroyWindow(window);
                    
                    SDL_DestroyWindow(SpriteExplorer);
                    SDL_DestroyTexture(SpriteExplorerTexture);
                    SDL_DestroyRenderer(SpriteExplorerrenderer);

                    running = false;
                    break;

                default:
                    break;
            }
            
        }

        if(RomLoaded){
            
            Update();     
            DrawViewport(renderer, texture);
            
            if(SpriteExplorer && triggered){

                HWND SpriteExplorerHandler = getSDLWinHandle(SpriteExplorer);

                SDL_Rect SpriteExplorerrect = { //current display at SCX: 0 and SCY: 0.
                0, 0, 8 * 21, 8 * 21};

                memset(SpriteExplorerDisplay, 0xE8, 256 * 256 * 3);

                switch(ViewSpriteSheet){
                    case 1:
                        LoadTilesFromMap();
                        break;
                    case 0:
                        LoadOAM();
                        break;
                    case 2:
                        LoadSpriteSheet();
                        break;
                } 

                SDL_RenderClear(SpriteExplorerrenderer);
                
                SDL_UpdateTexture(SpriteExplorerTexture, NULL, SpriteExplorerDisplay, 256 *3);
                
                SDL_RenderCopy(SpriteExplorerrenderer, 
                                SpriteExplorerTexture, 
                                (ViewSpriteSheet) ? NULL : &SpriteExplorerrect, 
                                NULL); //just display entire field for now

                SDL_RenderPresent(SpriteExplorerrenderer);


            }

        }

        int delta = SDL_GetTicks() - startLoop;
        DesiredDelta = 1000/fps;
        if(delta < DesiredDelta) SDL_Delay(DesiredDelta - delta);


    }

    return 0;
    
}