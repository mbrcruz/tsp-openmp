CCFLAGS=-fopenmp -O3 -DPRINT
LDFLAGS=-fopenmp -O3
CC=icpc

all: tsp-openmp

tsp-mpi: tsp-openmp.o
	$(CC) tsp-openmp.o  -o tsp-openmp  $(LDFLAGS)

tsp-openmp.o : tsp-openmp.c
	$(CC) -o tsp-openmp.o -c tsp-openmp.c $(CCFLAGS)

clean:
	rm tsp-openmp tsp-openmp.o
cleanr:
	rm OpenMP-*.o*
