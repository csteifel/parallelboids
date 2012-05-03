

all: boid main setup
	mpicc -Wall -o boids main.o boid.o setup.o -lm

setup: setup.c
	mpicc -c -Wall -o setup.o setup.c -lm
boid: boid.c
	mpicc -c -Wall -o boid.o boid.c -lm

main: main.c
	mpicc -c -Wall -o main.o main.c -lm

clean: 
	rm *.o
