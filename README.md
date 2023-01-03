# Fatboy! 🙏🏾

Fatboy was written for the purposes of learning about low-level programming, the C language and emulation in general for future projects.
It was project that around a month and a half to complete, starting on 11/19/23 and releasing the first official version on 1/3/23.
It Utilizes SDL2 for Visual and Auditory Output. A concurrent release of the SDL2.dll is included.

It was extremely difficult, but I learned about an abundance of topics: Compilation, Interpretation, Display Rendering, Digital Audio Processing,
Assembly Languages, General Software Development Workflow, etc. I'm very glad I decided to pursue this.

## Screenshots

Compatible with all these titles and perhaps more, with minor auditory and visual bugs in pokémon Red.

![DrMario](/res/dr%20Mario.PNG)
![Pokemon](/res/Pokemon.PNG )
![Tetris](/res/Tetris.PNG)
![Zelda](/res/Zelda.PNG)

## Installation

You can either download the latest release or build the source code with make.

```
output:
	gcc -I./src -ISDL2/Include -LSDL2/lib -o Fatboy src/*.c -lSDL2main -lSDL2 -lcomdlg32 -lgdi32
```

## Controls

| Keyboard | Gameboy |
| -------- | ------- |
| <kbd>Enter</kbd>     | Start   |
|<kbd>Shift</kbd>  | Select  |
| <kbd>↑</kbd>  | Up      |
|  <kbd>↓</kbd> | Down    |
|   <kbd>←</kbd> | Left    |
|   <kbd>→</kbd>  | Right   |
|    <kbd>X</kbd>  | A      |
|     <kbd>Z</kbd>     | B      |
|    <kbd>+</kbd>  | Increase Volume      |
|     <kbd>-</kbd>     | Decrease Volume      |

## TODO

- [x] CPU instruction emulation
- [x] Audio emulation
- [x] BG & OBJ Scanline Renderer
- [x] Spritesheet, OAM, and TileMap display in View Tab
- [ ] Accurate Sprite Priority Rendering
- [ ] Accurate Instruction Timing 
- [ ] Accurate Window Scrolling


## Testing

There is still work to be done in clearing up inaccuracies in my HALT opcode.
![Test](/res/Test.PNG)

## Historic README file
[Notes.md](/Notes.md)

## Reference

* [Pan Docs](https://gbdev.io/pandocs/About.html)
* [http://www.codeslinger.co.uk/pages/projects/gameboy/beginning.html](http://www.codeslinger.co.uk/pages/projects/gameboy/beginning.html)
* [Nightshade's Blog](https://nightshade256.github.io/2021/03/27/gb-sound-emulation.html)
* [EmuDev Reddit & Discord](https://www.reddit.com/r/EmuDev/comments/9mop2q/join_the_official_remudev_chat_on_discord/)
* [Cinoop Emulator by CTurt](https://github.com/CTurt/Cinoop)
* [RealBoy](https://realboyemulator.wordpress.com/)
* [Gekkio's Game Boy: Complete Technical Reference](https://github.com/Gekkio/gb-ctr)