
output:
	gcc -I./src -ISDL2/Include -LSDL2/lib -o Fatboy src/*.c -lSDL2main -lSDL2 -lcomdlg32 -lgdi32
