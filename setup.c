#include "inc.h"

//Position boids on the field based on what is passed in via file (regular text file)
//First line should be maximum sizes (width height)
/*
file encoding: 0 - barrier/unusable spot
1 - free spot open on the board
2 - boid in location
3 - exit location
*/
void setupSimulation(char * fileName, boidContainer * boids, short *** board, unsigned int * width, unsigned int * height){
	FILE * fd = fopen(fileName, "r");
	unsigned int i, j;
	boid * newBoid = NULL;

	fscanf(fd, "%u %u", width, height);

	(*board) = (short **) calloc(*width, sizeof(short *));
	for(i = 0; i < *width; i++){
		(*board)[i] = (short *) calloc(*height, sizeof(short));
	}
	
	//Go through parsing the file
	for(j = 0; j < *height; j++){
		for(i = 0; i < *width; i++){
			printf("%d\n", fscanf(fd, "%1hd", &((*board)[i][j])));
			printf("%u %hd\n", i, (*board)[i][j]);
			if((*board)[i][j] == 2){
				(*board)[i][j] = 1;
				newBoid = (boid *) calloc(1, sizeof(boid));
				newBoid->xpos = i;
				newBoid->ypos = j;
				newBoid->active = 1;
				newBoid->velocity.x = 0;
				newBoid->velocity.y = 0;
				boidInsert(boids, newBoid);
				newBoid = NULL;
			}else if((*board)[i][j] == 3){
				//Should add to a list of exits so that its easy to figure out closest exit don't need to go through entire map
				
			}
		}
	}
}

