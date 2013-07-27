all: brainfuck.o

%.o : %.c
	gcc -O2 -g3 --std=gnu99 -o $@ $<
