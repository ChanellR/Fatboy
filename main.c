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
#define IDM_FILE_NEW 1
#define IDM_FILE_OPEN 2
#define IDM_FILE_QUIT 3

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

void CreateMenuBar (HWND hwnd)
{

    HMENU hMenubar;
    HMENU hMenu;

    hMenubar = CreateMenu();
    hMenu = CreateMenu();


    AppendMenuW(hMenu, MF_STRING, IDM_FILE_OPEN, L"&Open");
    AppendMenuW(hMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(hMenu, MF_STRING, IDM_FILE_QUIT, L"&Quit");

    AppendMenuW(hMenubar, MF_POPUP, (UINT_PTR) hMenu, L"&File");
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
  }

}


void DrawViewport (SDL_Renderer* renderer, SDL_Texture* texture)
{
    int W = WindowWidth;
    int H = WindowHeight;

    SDL_RenderClear(renderer);
    SDL_UpdateTexture(texture, NULL, DisplayPixels, 256*3); //changed for logo
    

    
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



int main(int argc, char** argv){

    int fps = 60;
    int DesiredDelta = 1000 / fps;


    LoadRom("panda.gb");
    Reset();
    //remove("Log.txt");

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
                    
                    if (WM_MESSAGE(event).msg == WM_CREATE) ;

                    
                    if (WM_MESSAGE(event).msg == WM_COMMAND)
                    {
                        switch (LOWORD(WM_MESSAGE(event).wParam))
                        {

                            case IDM_FILE_OPEN:

                                MessageBeep(MB_ICONINFORMATION);
                                OpenDialog(WindowHandler);
                                break;
                                
                            break;
                  
                            case IDM_FILE_QUIT:
                            
                                SendMessage(WindowHandler, WM_CLOSE, 0, 0);
                                break; 
                        }
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
        

        DrawViewport(renderer, texture);
        Update(); //assuming this takes an insignificant amount of time
        LoadTilesFromMap();
        int delta = SDL_GetTicks() - startLoop;
        if(delta < DesiredDelta) SDL_Delay(DesiredDelta - delta);


    }

    return 0;
    
}