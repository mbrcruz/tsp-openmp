GCC=gcc
CCFLAGS=-O3
LDFLAGS=-lm -O3
CC=gcc

all: tsp-openmp

tsp-mpi: tsp-openmp.o
	$(CC) -o tsp-openmp tsp-openmp.o $(LDFLAGS)

tsp-mpi.o : tsp-openmp.c
	$(CC) -o tsp-openmp.o -c tsp-openmp.c $(CCFLAGS)

cleanall:
	rm tsp-openmp tsp-openmp.o
