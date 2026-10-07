CC = gcc
CFLAGS = -Wall -pedantic -std=gnu99 -Wextra

all: brainfuck

.PHONY: clean

brainfuck: main.c
	cc main.c -o brainfuck

clean:
	rm -f solver *.o
	
