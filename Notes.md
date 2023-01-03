# OLDREADME

I've decided to develop a gameboy emulator in order to become more engaged in the subject of emulation and eventually move on to contributing to projects like Yuzu and Ryujinx.

There's not much now, but there is certainly a backbone for everything to come to fruition. I've decided to write it in C in order to learn the language, as well as set myself up to work with C++ on other projects.

As of Now, I have about half of the instruction set implemented.

11/25/22

I decided to rewrite my code after seeing some other types of implementations and have come to find it much easier to work in this format. Even though I've done a majority of the work before, and I am often moving code, it still feels much easier to do what I want to do, and I'm glad I decided to shift things.

11/27/22

CINOOP's realtime debugger has taught me a lot about the importance of debuggers. It's very interesting to see how necessary it is to write code that tests your code. I've heard of debugging and tests that are much longer than individual pieces of code, and that developers mainly spend time testing code rather than running it. That may or may not be true but it's definitely conceivable.

12/12/22

After having finally passed all of Blargg's cpu instruction tests, I realized how beneficial it would've been for me to put effort into finding out how to work them in the beginning and use them to track my progress. When I finished my cpu implementation without testing and moved on to unsuccessfully implementing the PPU, it made me really discouraged because I felt as though I would never get it to work. Having this structure, something that can empirically track my progress has done wonders for me.

12/17/22

Tetris and Dr. Mario is now completely playable. I need to do sound and fix timing issues along with work out some visual glitches but everything is alright, because at last you can play the game.



## Start of Notes

gcc -Isrc/Include -Lsrc/lib -o Fatboy main.c cpu.c ppu.c apu.c memory.c -lSDL2main -lSDL2 -lcomdlg32 -lgdi32
PC: 00:4047 (C9 3E 01 EA)
Notes:
    pass the loop by fixing LY  
    Read codeslinger to really figure out where it reads from 
    go through the tests
    Take a nice break. 


Parts:
    Instruction set:
        reading and accurately recognizing instructions
        functioning incstructions that are up to par with an actual machine
        I still need to factor in cycles when the jumps don't go through
    Timing:
        Can be synced up with the cycle processing to emulate true Timing
        should always just be working depending on the settings, not hard to implement
    Interrupts:
        I have the destinations for jumps active but I need to be able to recognize the 
            changes acutally. Make sure the enable's function. 
    LCD:
        Need to understand better how the modes(vblank, etc.) actually function and what
            everything else should be doing. 


First mode of agenda:
    Fix the Ly to make sure it's scaling when it's supposed to be. 

Stat:
    searching OAM: 38 cnt
    Loading to LCD: 76 cnt, (actually drawing)
    Hblank: 100 cnt, 
    Vblank: 



at 2004, they have 1F8B stored in Stack, I may or may not have that in the stack 
I have no way of determining that


----------------12/5 issues

Ly not decreasing fast enough, 
may be calling LCD stat too quickly without refreshing with joystick or vblank
Find when it gets stuck
can scy be greater than 144, maybe
is TAC supposed to be on, 
fix the display code, 
the 39 is probably because things are getting caught on the 
rest 38. 

----------------------12/12 

Blargg tests
ALL PASSED

Now we move to testing timing.

01:255-3 02:254-2 03:254-2 06:254-2 09:254-2 0A:254-2 0B:254-2 0E:254-2 11:255-3 12:254-2 13:254-2 16:254-2 18:255-3 19:254-2 1A:254-2 1B:254-2 1E:254-2 20:254-2 20:255-3 21:255-3 22:254-2 23:254-2 26:254-2 28:254-2 28:255-3 29:254-2 2A:254-2 2B:254-2 2E:254-2 30:254-2 30:255-3 31:255-3 32:254-2 33:254-2 34:1-3 35:1-3 36:254-3 38:254-2 38:255-3 39:254-2 3A:254-2 3B:254-2 3E:254-2 46:1-2 4E:1-2 56:1-2 5E:1-2 66:1-2 6E:1-2 70:1-2 71:1-2 72:1-2 73:1-2 74:1-2 75:1-2 77:1-2 7E:1-2 86:1-2 8E:1-2 96:1-2 9E:1-2 A6:1-2 AE:1-2 B6:1-2 BE:1-2 C0:254-2 C1:255-3 C2:255-3 C4:255-3 C4:2-6 C6:254-2 C8:254-2 CA:255-3 CC:255-3 CC:2-6 CD:2-6 CE:1-2 D0:254-2 D1:255-3 D2:255-3 D4:255-3 D4:2-6 D6:254-2 D8:254-2 DA:255-3 DC:255-3 DC:2-6 DE:254-2 E0:255-3 E1:255-3 E2:254-2 E6:254-2 EE:254-2 F0:255-3 F1:255-3 F2:5-2 F2:0-2 F6:254-2 F8:255-3 F9:254-2 FE:254-2 CB 00:254-2 CB 01:254-2 CB 02:254-2 CB 03:254-2 CB 04:254-2 CB 05:254-2 CB 06:254-4 CB 07:254-2 CB 08:254-2 CB 09:254-2 CB 0A:254-2 CB 0B:254-2 CB 0C:254-2 CB 0D:254-2 CB 0E:254-4 CB 0F:254-2 CB 10:254-2 CB 11:254-2 CB 12:254-2 CB 13:254-2 CB 14:254-2 CB 15:254-2 CB 16:254-4 CB 17:254-2 CB 18:254-2 CB 
19:254-2 CB 1A:254-2 CB 1B:254-2 CB 1C:254-2 CB 1D:254-2 CB 1E:254-4 CB 1F:254-2 CB 20:254-2 CB 21:254-2 CB 22:254-2 CB 23:254-2 CB 24:254-2 CB 25:254-2 CB 26:254-4 CB 27:254-2 CB 28:254-2 CB 29:254-2 CB 2A:254-2 CB 2B:254-2 CB 2C:254-2 CB 2D:254-2 CB 2E:254-4 CB 2F:254-2 CB 30:254-2 CB 31:254-2 CB 32:254-2 CB 33:254-2 CB 34:254-2 CB 35:254-2 CB 36:254-4 CB 37:254-2 CB 38:254-2 CB 39:254-2 CB 3A:254-2 CB 3B:254-2 CB 3C:254-2 CB 3D:254-2 CB 3E:254-4 CB 3F:254-2 CB 40:254-2 CB 41:254-2 CB 
42:254-2 CB 43:254-2 CB 44:254-2 CB 45:254-2 CB 46:254-3 CB 47:254-2 CB 48:254-2 CB 49:254-2 CB 4A:254-2 CB 4B:254-2 CB 4C:254-2 CB 4D:254-2 CB 4E:254-3 CB 4F:254-2 CB 50:254-2 CB 51:254-2 CB 52:254-2 CB 53:254-2 CB 54:254-2 CB 55:254-2 CB 56:254-3 CB 57:254-2 CB 58:254-2 CB 59:254-2 CB 5A:254-2 CB 5B:254-2 CB 5C:254-2 CB 5D:254-2 CB 5E:254-3 CB 5F:254-2 CB 60:254-2 CB 61:254-2 CB 62:254-2 CB 63:254-2 CB 64:254-2 CB 65:254-2 CB 66:254-3 CB 67:254-2 CB 68:254-2 CB 69:254-2 CB 6A:254-2 CB 
6B:254-2 CB 6C:254-2 CB 6D:254-2 CB 6E:254-3 CB 6F:254-2 CB 70:254-2 CB 71:254-2 CB 72:254-2 CB 73:254-2 CB 74:254-2 CB 75:254-2 CB 76:254-3 CB 77:254-2 CB 78:254-2 CB 79:254-2 CB 7A:254-2 CB 7B:254-2 CB 7C:254-2 CB 7D:254-2 CB 7E:254-3 CB 7F:254-2 CB 80:254-2 CB 81:254-2 CB 82:254-2 CB 83:254-2 CB 84:254-2 CB 85:254-2 CB 86:254-4 CB 87:254-2 CB 88:254-2 CB 89:254-2 CB 8A:254-2 CB 8B:254-2 CB 8C:254-2 CB 8D:254-2 CB 8E:254-4 CB 8F:254-2 CB 90:254-2 CB 91:254-2 CB 92:254-2 CB 93:254-2 CB 
94:254-2 CB 95:254-2 CB 96:254-4 CB 97:254-2 CB 98:254-2 CB 99:254-2 CB 9A:254-2 CB 9B:254-2 CB 9C:254-2 CB 9D:254-2 CB 9E:254-4 CB 9F:254-2 CB A0:254-2 CB A1:254-2 CB A2:254-2 CB A3:254-2 CB A4:254-2 CB A5:254-2 CB A6:254-4 CB A7:254-2 CB A8:254-2 CB A9:254-2 CB AA:254-2 CB AB:254-2 CB AC:254-2 CB AD:254-2 CB AE:254-4 CB AF:254-2 CB B0:254-2 CB B1:254-2 CB B2:254-2 CB B3:254-2 CB B4:254-2 CB B5:254-2 CB B6:254-4 CB B7:254-2 CB B8:254-2 CB B9:254-2 CB BA:254-2 CB BB:254-2 CB BC:254-2 CB 
BD:254-2 CB BE:254-4 CB BF:254-2 CB C0:254-2 CB C1:254-2 CB C2:254-2 CB C3:254-2 CB C4:254-2 CB C5:254-2 CB C6:254-4 CB C7:254-2 CB C8:254-2 CB C9:254-2 CB CA:254-2 CB CB:254-2 CB CC:254-2 CB CD:254-2 CB CE:254-4 CB CF:254-2 CB D0:254-2 CB D1:254-2 CB D2:254-2 CB D3:254-2 CB D4:254-2 CB D5:254-2 CB D6:254-4 CB D7:254-2 CB D8:254-2 CB D9:254-2 CB DA:254-2 CB DB:254-2 CB DC:254-2 CB DD:254-2 CB DE:254-4 CB DF:254-2 CB E0:254-2 CB E1:254-2 CB E2:254-2 CB E3:254-2 CB E4:254-2 CB E5:254-2 CB 
E6:254-4 CB E7:254-2 CB E8:254-2 CB E9:254-2 CB EA:254-2 CB EB:254-2 CB EC:254-2 CB ED:254-2 CB EE:254-4 CB EF:254-2 CB F0:254-2 CB F1:254-2 CB F2:254-2 CB F3:254-2 CB F4:254-2 CB F5:254-2 CB F6:254-4 CB F7:254-2 CB F8:254-2 CB F9:254-2 CB FA:254-2 CB FB:254-2 CB FC:254-2 CB FD:254-2 CB FE:254-4 CB FF:254-2 
Failed

------------------12/14

leaving timing, working towards actually scanline printing
HUD, menus.


--12/16
A: 32 F: 00 B: 00 C: 8A D: 12 E: 30 H: 64 L: F8 SP: DFF5 PC: 00:614D (00 00 00 1B)
A: 32 F: 00 B: 00 C: 8A D: 12 E: 30 H: 64 L: F8 SP: DFF5 PC: 00:614E (00 00 1B 7A)
A: 32 F: 00 B: 00 C: 8A D: 12 E: 30 H: 64 L: F8 SP: DFF5 PC: 00:614F (00 1B 7A B3)
A: 32 F: 00 B: 00 C: 8A D: 12 E: 30 H: 64 L: F8 SP: DFF5 PC: 00:6150 (1B 7A B3 20)
A: 32 F: 00 B: 00 C: 8A D: 12 E: 2F H: 64 L: F8 SP: DFF5 PC: 00:6151 (7A B3 20 F8)
A: 12 F: 00 B: 00 C: 8A D: 12 E: 2F H: 64 L: F8 SP: DFF5 PC: 00:6152 (B3 20 F8 C9)
A: 3F F: 00 B: 00 C: 8A D: 12 E: 2F H: 64 L: F8 SP: DFF5 PC: 00:6153 (20 F8 C9 FA)

loop in pokemon;


DR Mario infinite loop:
A: 01 F: 20 B: 00 C: 00 D: 00 E: 09 H: 99 L: 71 SP: CFF7 PC: 00:0B7B (F0 41 E6 03)
A: 81 F: 20 B: 00 C: 00 D: 00 E: 09 H: 99 L: 71 SP: CFF7 PC: 00:0B7D (E6 03 20 FA)
A: 01 F: 20 B: 00 C: 00 D: 00 E: 09 H: 99 L: 71 SP: CFF7 PC: 00:0B7F (20 FA F1 C9)
A: 01 F: 20 B: 00 C: 00 D: 00 E: 09 H: 99 L: 71 SP: CFF7 PC: 00:0B7B (F0 41 E6 03)
A: 81 F: 20 B: 00 C: 00 D: 00 E: 09 H: 99 L: 71 SP: CFF7 PC: 00:0B7D (E6 03 20 FA)
A: 01 F: 20 B: 00 C: 00 D: 00 E: 09 H: 99 L: 71 SP: CFF7 PC: 00:0B7F (20 FA F1 C9)



SHIFT WINDOW BY SCY AS WELL SO IT FITS ON SCREEN

zelda crashes on 1F to the left and 1B to the right

The gameboy PPU works from left to right, drawing background and then sprites as it goes - but it doesn't draw them as colours, it draws them as a palette index. This lets it distinguish sprite pixels from bg ones, and lets it know when a bg pixel is palette index 0 for the "draw under bg" function (because the sprite is actually drawn after the bg)
It then resolves the palette indexes into actual colours when the pixels are actually output
Because it doesn't know sprite priority of already drawn pixels, only whether they are sprite 0/1 or bg palette, you get the odd effect that a "under bg" sprite at X=1 will actually draw on top of an "over bg" sprite at X=2
But to get that correct you have to draw the left sprite first - because it only draws on top of pixels where the bg has palette 0
not on top of all of the pixels of sprite 2
Example:
BG: 11100111
S1:  2222222 under bg
S2:   333333 over bg
Out:11322333

Sprite 1 is drawn in the BG's 0 spots, then sprite 2 draws over the bg pixels that are left

1/3/23
Submitted the Offical README.md
