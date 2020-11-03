CC=gcc
CFLAGS=-Wall
DEPS=utils.h utils.c
PROG=genPoints

VPATH=src

all: genPoints

genPoints: genPoints.c $(DEPS)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm $(PROG)
