

all: boid main setup
	mpicc -Wall  -pthread -o boids main.o boid.o setup.o

setup: setup.c
	mpicc -c -Wall  -pthread -o setup.o setup.c

boid: boid.c
	mpicc -c -Wall  -pthread -o boid.o boid.c

main: main.c
	mpicc -c -Wall  -pthread -o main.o main.c

clean: 
	rm *.o
