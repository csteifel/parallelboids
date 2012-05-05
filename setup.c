#include "inc.h"

//Position boids on the field based on what is passed in via file (regular text file)
//First line should be maximum sizes (width height)
/*
file encoding: 0 - barrier/unusable spot
1 - free spot open on the board
2 - boid in location
3 - exit location
*/
void setupSimulation(char * fileName, boidContainer * boids, wallContainer * walls, goalContainer * goals, short *** board, short *** blank, unsigned int * width, unsigned int * height){
	MPI_File fd;
	unsigned int i, j;
	unsigned int buf[2];
	short * readArr;
	boid * newBoid = NULL;
	boidWall * newWall = NULL;

	//Open the file throughout all processors
	MPI_File_open(MPI_COMM_WORLD, fileName, MPI_MODE_RDONLY, MPI_INFO_NULL, &fd);

	//MUST USE BINARY FILES FOR MPI VERSION
	MPI_File_read_all(fd, buf, 2, MPI_UNSIGNED, MPI_STATUS_IGNORE);


	*width= buf[0];
	*height = buf[1];


	widthSlice = *width;
	heightSlice = *height / numranks;
	widthOffset = 0;
	heightOffset = heightSlice * rank;


	(*board) = (short **) calloc(*width, sizeof(short *));
	(*blank) = (short **) calloc(*width, sizeof(short *));
	for(i = 0; i < *width; i++){
		(*board)[i] = (short *) calloc(*height, sizeof(short));
		(*blank)[i] = (short *) calloc(*height, sizeof(short));
	}

	readArr = (short *) calloc(*width, sizeof(short));

	//Read in the entire board but only add boids that we are looking out for to our list of boids
	//Go through parsing the file
	for(j = 0; j < *height; j++){
		MPI_File_read_all(fd, readArr, (int) (*width), MPI_SHORT, MPI_STATUS_IGNORE);
		for(i = 0; i < *width; i++){
			(*board)[i][j] = readArr[i];
			(*blank)[i][j] = readArr[i];
			if((*board)[i][j] == 2 ){
				(*blank)[i][j] = 1;
				if(j >= heightOffset && j < heightOffset+heightSlice){
					newBoid = (boid *) calloc(1, sizeof(boid));
					newBoid->xpos = i;
					newBoid->ypos = j;
					newBoid->active = 1;
					newBoid->velocity.x = 0;
					newBoid->velocity.y = 0;
					boidInsert(boids, newBoid);
					newBoid = NULL;
				}else{
					(*board)[i][j] = 1;
				}
			}
			if((*board)[i][j] == 3){
				//Should add to a list of exits so that its easy to figure out closest exit don't need to go through entire map
				addGoal(goals, i, j);
			}
			if((*board)[i][j] == 4) // add walls to board
			{
			  newWall = (boidWall *) calloc(1, sizeof(boidWall));
			  newWall->xpos = i;
			  newWall->ypos = j;
			  wallInsert(walls,newWall);
			}
		}
	}
	MPI_File_close(&fd);
}

void addGoal(goalContainer * goals, int x, int y){
	//Check for need to extend container
	if(goals->size == goals->alloc){
		goals->alloc += CONTAINEREXTEND;
		
		if((goals->pos = (int **) realloc(goals->pos, goals->alloc * sizeof(int *))) == NULL){
			fprintf(stderr, "Error extending goal conatiner, quitting...\n");
			exit(1);
		}
	}

	goals->pos[goals->size] = (int *) calloc(2, sizeof(int));
	goals->pos[goals->size][0] = x;
	goals->pos[goals->size][1] = y;
	++goals->size;
}
