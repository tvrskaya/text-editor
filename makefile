CFLAGS= -Wall -Wextra -std=c11 -pedantic -ggdb `pkg-config --cflags sdl2`
CC=gcc
LIBS=`pkg-config --libs sdl2` -lm

te: main.c 
	$(CC) $(CFLAGS) main.c la.c -omain.exe $(LIBS)
