CXX=mpicxx
CPPFLAGS=-Wall -O3
LDFLAGS=

PROG=genPoints parallelSearch processPool forkJoin partitionSpace

VPATH=src

all: $(PROG)

genPoints: genPoints.o utils.o
	$(CXX) $(LDFLAGS) -lm -o $@ $^

genPoints.o: genPoints.c
	$(CXX) $(CPPFLAGS) -c $<

parallelSearch: parallelSearch.o utils.o
	$(CXX) $(LDFLAGS) -o $@ $^

parallelSearch.o: parallelSearch.c
	$(CXX) $(CPPFLAGS) -c $<

processPool: processPool.o utils.o
	$(CXX) $(LDFLAGS) -o $@ $^

processPool.o: processPool.c processPool.h
	$(CXX) $(CPPFLAGS) -c $<

forkJoin: forkJoin.o utils.o
	$(CXX) $(LDFLAGS) -o $@ $^

forkJoin.o: forkJoin.cpp
	$(CXX) $(CPPFLAGS) -c $<

partitionSpace: partitionSpace.o circular_linked_list.o utils.o
	$(CXX) $(LDFLAGS) -o $@ $^

partitionSpace.o: partitionSpace.cpp
	$(CXX) $(CPPFLAGS) -c $<

circular_linked_list.o: circular_linked_list.cpp circular_linked_list.hpp
	$(CXX) $(CPPFLAGS) -c $<

utils.o: utils.c utils.h
	$(CXX) $(CPPFLAGS) -c $<



clean:
	rm -f $(PROG) *.o
