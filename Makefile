

all: boid main setup
	gcc -Wall  -pthread -o boids main.o boid.o setup.o

setup: setup.c
	gcc -c -Wall  -pthread -o setup.o setup.c
boid: boid.c
	gcc -c -Wall  -pthread -o boid.o boid.c

main: main.c
	gcc -c -Wall  -pthread -o main.o main.c

clean: 
	rm *.o
