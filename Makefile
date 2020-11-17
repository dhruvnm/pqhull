CC=mpicxx
CFLAGS=-Wall -O3
DEPS=utils.h utils.c
PROG=genPoints parallelSearch processPool

VPATH=src

all: $(PROG)

genPoints: genPoints.c $(DEPS)
	$(CC) $(CFLAGS) -lm -o $@ $^

parallelSearch: parallelSearch.c $(DEPS)
	$(CC) $(CFLAGS) -o $@ $^

processPool: processPool.c processPool.h $(DEPS)
	$(CC) $(CFLAGS) -o $@ $^

forkJoin: forkJoin.c $(DEPS)
	$(CC) $(CFLAGS) -o $@ $^


clean:
	rm $(PROG)
