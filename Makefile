

all: boid main setup
	gcc -Wall -o boids main.o boid.o setup.o

setup: setup.c
	gcc -c -Wall -o setup.o setup.c
boid: boid.c
	gcc -c -Wall -o boid.o boid.c

main: main.c
	gcc -c -Wall -o main.o main.c

clean: 
	rm *.o
