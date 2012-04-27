

all: boid main
	gcc -Wall -o boids main.o boid.o

boid: boid.c
	gcc -c -Wall -o boid.o boid.c

main: main.c
	gcc -c -Wall -o main.o main.c

clean: 
	rm *.o
