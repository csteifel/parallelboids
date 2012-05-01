

all: boid main setup
	mpicc -Wall -o boids main.o boid.o setup.o

setup: setup.c
	mpicc -c -Wall -o setup.o setup.c
boid: boid.c
	mpicc -c -Wall -o boid.o boid.c

main: main.c
	mpicc -c -Wall -o main.o main.c

clean: 
	rm *.o
