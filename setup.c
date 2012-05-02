#include "inc.h"

//Position boids on the field based on what is passed in via file (regular text file)
//First line should be maximum sizes (width height)
/*
file encoding: 0 - barrier/unusable spot
1 - free spot open on the board
2 - boid in location
3 - exit location
*/
void setupSimulation(char * fileName, boidContainer * boids, goalContainer * goals, short *** board, short *** blank, unsigned int * width, unsigned int * height){
	MPI_File fd;
	unsigned int i, j;
	unsigned int buf[2];
	int across;
	short * readArr = NULL;
	boid * newBoid = NULL;

	//Open the file throughout all processors
	MPI_File_open(MPI_COMM_WORLD, fileName, MPI_MODE_RDONLY, MPI_INFO_NULL, &fd);

	//MUST USE BINARY FILES FOR MPI VERSION
	MPI_File_read_all(fd, buf, 2, MPI_UNSIGNED, MPI_STATUS_IGNORE);

	*width = buf[0];
	*height = buf[1];
	

	//Get how many processors go across the top and down therefore making it easy to calculate area to watch
	//world size has to be a power of 2
	across = (int) sqrt( worldSize);
	row = across/myRank;
	column = across % myRank;

	//Unfortunately if we want to doing map[x][y] we have to initialize everything first because reading is [y][x]
	(*board) = (short **) calloc(*width, sizeof(short *));
	(*blank) = (short **) calloc(*width, sizeof(short *));
	for(i = 0; i < *width; i++){
		(*board)[i] = (short *) calloc(*height, sizeof(short));
		(*blank)[i] = (short *) calloc(*height, sizeof(short));
	}
	
	readArr = (short *) calloc(*width, sizeof(short));
	
	//Go through parsing the file
	for(j = 0; j < *height; j++){
		MPI_File_read_all(fd, buf, (int) (*width), MPI_SHORT, MPI_STATUS_IGNORE);
		for(i = 0; i < *width; i++){

			(*blank)[i][j] = readArr[i];

			if((*board)[i][j] == 2 && i >= column*((*width)/across) && i < (column+1)*((*width)/across) && j >= row*((*height)/across) && j < (row+1)*((*height)/across) ){
				(*blank)[i][j] = 1;

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
				addGoal(goals, i, j);
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
