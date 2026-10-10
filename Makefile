CC = gcc
CFLAGS = -Wall -pedantic -std=gnu99 -Wextra

all: brainfuck

.PHONY: clean

brainfuck: main.c # stack.o
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f solver *.o
	
