# Fatboy! 🙏🏾

Fatboy was written for the purposes of learning about low-level programming, the C language and emulation in general for future projects.
It took around a month and a half to complete, starting on 11/19/22 and releasing the first official version on 1/3/23.
It utilizes SDL2 for visual and auditory output. A currently up to date release of the SDL2.dll is included.

It was extremely difficult, but I learned about an abundance of Computer Enginnering topics: Compilation, Interpretation, Display Rendering, Digital Audio Processing,
Assembly Languages, General Software Development Workflow, etc. I'm very glad I decided to pursue this.

## Screenshots

Compatible with all these titles and perhaps more, with minor auditory and visual bugs in pokémon Red.

<img src="/res/dr%20Mario.PNG" width="350" height="350">  <img src="/res/Pokemon.PNG " width="350" height="350">
<img src="/res/Tetris.PNG" width="350" height="350">  <img src="/res/Zelda.PNG" width="350" height="350">

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
- [x] Saving and File IO
- [ ] Accurate Sprite Priority Rendering
- [ ] Accurate Instruction Timing 
- [ ] Accurate Window Scrolling


## Testing

There is still work to be done in clearing up inaccuracies in my HALT opcode.

<img src="/res/Test.PNG" width="350" height="350">

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
