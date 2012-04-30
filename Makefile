

all: boid main setup
	gcc -Wall -pg -pthread -o boids main.o boid.o setup.o

setup: setup.c
	gcc -c -Wall -pg -pthread -o setup.o setup.c
boid: boid.c
	gcc -c -Wall -pg -pthread -o boid.o boid.c

main: main.c
	gcc -c -Wall -pg -pthread -o main.o main.c

clean: 
	rm *.o
