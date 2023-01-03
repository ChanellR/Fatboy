# Fatboy

Fatboy was written for the purposes of learning about low-level programming and emulation in general for future projects.
It was project that around a month and a half to complete, starting on 11/19/23 and releasing the first official version on 1/3/23.
It Utilizes SDL2 for Visual and Auditory Output.

![https://github.com/HFO4/gameboy.live/raw/master/doc/screenshot.png](https://github.com/HFO4/gameboy.live/raw/master/doc/screenshot.png)

## Installation

You can directly download the executable file from the [Release](https://github.com/HFO4/gameboy.live/releases) page, or build it from the source. Go Version 1.11 or higher is required. Run `go version` to check what the version currently installed is. On Debian based systems, the packages `libasound2-dev` and `libgl1-mesa-dev` must be installed.

```
git clone https://github.com/HFO4/gameboy.live.git
cd gameboy.live
go build -o gbdotlive main.go
```

## Controls

| Keyboard | Gameboy |
| -------- | ------- |
| <kbd>Enter</kbd>     | Start   |
|<kbd>Backspace</kbd>  | Select  |
| <kbd>↑</kbd>  | Up      |
|  <kbd>↓</kbd> | Down    |
|   <kbd>←</kbd> | Left    |
|   <kbd>→</kbd>  | Right   |
|    <kbd>X</kbd>  | A      |
|     <kbd>Z</kbd>     | B      |

## Features & TODOs

- [x] CPU instruction emulation
- [x] Timer and interrupt
- [x] Support for ROM-only, MBC1, MBC2, MBC3 cartridge
- [x] Sound emulation
- [x] Graphics emulation
- [x] Cloud gaming
- [x] ROM debugger
- [x] Game saving & restore in cartridge level

There are still many TODOs：

- [ ] Support Gameboy Color emulation


## Testing

![Testing result](https://github.com/HFO4/gameboy.live/raw/master/doc/Testing.jpg)

## Reference

* [Pan Docs](http://bgb.bircd.org/pandocs.htm)
* [http://www.codeslinger.co.uk/pages/projects/gameboy/beginning.html](http://www.codeslinger.co.uk/pages/projects/gameboy/beginning.html)
* [http://www.devrs.com/gb/files/GBCPU_Instr.html](http://www.devrs.com/gb/files/GBCPU_Instr.html)
* [https://github.com/Humpheh/goboy](https://github.com/Humpheh/goboy)
* [The Ultimate Game Boy Talk (33c3)](https://www.youtube.com/watch?v=HyzD8pNlpwI)
* [http://gameboy.mongenel.com/dmg/asmmemmap.html](http://gameboy.mongenel.com/dmg/asmmemmap.html)
* ......