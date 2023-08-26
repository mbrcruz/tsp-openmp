CCFLAGS=-O3
LDFLAGS=-L/usr/lib/x86_64-linux-gnu/ -lm  -O3
CC=gcc

debug: tsp-openmp
CCFLAGS=-g
LDFLAGS=-L/usr/lib/x86_64-linux-gnu/ -lm -g

all: tsp-openmp

tsp-mpi: tsp-openmp.o
	$(CC) -o tsp-openmp tsp-openmp.o $(LDFLAGS)

tsp-mpi.o : tsp-openmp.c
	$(CC) -o tsp-openmp.o -c tsp-openmp.c $(CCFLAGS)

cleanall:
	rm tsp-openmp tsp-openmp.o
