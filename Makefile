all:
	gcc -Isrc/Include -Lsrc/lib -o main main.c display.c control.c cpu.c gpu.c memory.c -lmingw32 -lSDL2main -lSDL2