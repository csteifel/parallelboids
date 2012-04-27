#include "inc.h"

//Position boids on the field based on what is passed in via file (regular text file)
//First line should be maximum sizes (width height)
/*
file encoding: 0 - barrier/unusable spot
1 - free spot open on the board
2 - boid in location
3 - exit location
*/
void setupSimulation(char * fileName, boidContainer * boids, int ** board){
	FILE * fd = fopen(fileName, "r");
	unsigned int width, height, i;
	fscanf(fd, "%u %u", &width, &height);

	board = (int **) calloc(width, sizeof(int *));
	for(i = 0; i < width; i++){
		board[i] = (int *) calloc(height, sizeof(int));
	}
	
	
}
