CCFLAGS=-O3
LDFLAGS=-L/usr/lib/x86_64-linux-gnu/ -lm  -O3
CC=gcc

all: tsp-openmp

tsp-mpi: tsp-openmp.o
	$(CC) tsp-openmp.o  -o tsp-openmp $(LDFLAGS)

tsp-openmp.o : tsp-openmp.c
	$(CC) -o tsp-openmp.o -c tsp-openmp.c $(CCFLAGS)

cleanall:
	rm tsp-openmp tsp-openmp.o
