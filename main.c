#include "inc.h"

//Boid algorithm threaded approach

//Start out with a limit on iterations until goal achievement is programmed
#define ITERATIONS 9000 
#define NUMTHREADS 20


//Find the closest open spot on the map and place the boid there
void findClosest(short *** map, int x, int y, int width, int height, int * positions){
	int lookRadius = 1;
	int i, j, placed = 0;

	while(lookRadius < 2){
		i = x - lookRadius;
		i = (i < 0) ? 0 : i;
		for(; i <= x + lookRadius && i < width; i++){
			j = y - lookRadius;
			j = (j < 0) ? 0 : j;
			for(; j <= y + lookRadius && j < width; j++){
				if((*map)[i][j] == 1){
					positions[0] = i;
					positions[1] = j;
					placed = 1;
				}
			}
		}
		if(placed) break;
		lookRadius++;
	}
}


/*void * threadedMove(void * arg){
	struct arguments * args = (struct arguments *) arg;
	int i; 
	size_t count = 0;
	for(i = args->start; i < args->finish; i++){
		if(args->boidlist->boidArr[i].active){
			moveBoid(args->goals, args->boidlist, i);
			count++;
		}
	}

	pthread_exit((void *) count);
	return NULL;
}
*/

int step(boidContainer * boidlist, goalContainer * goals, short *** map, short *** blankMap, int width, int height){
	int i, count = 0;



	for(i = 0; i < width; i++){
		free((*map)[i]);
		(*map)[i] = (short *) calloc(height, sizeof(short));
		memcpy((*map)[i], (*blankMap)[i], height * sizeof(short));
	}


	//We need to reconcile the map now based on boid positions and check for people on top of each other
	for(i = 0; i < boidlist->size; i++){
		int location[2];

		if(boidlist->boidArr[i].active == 1){
			if((*map)[boidlist->boidArr[i].xpos][boidlist->boidArr[i].ypos] == 1){
				(*map)[boidlist->boidArr[i].xpos][boidlist->boidArr[i].ypos] = 2;
			}else if((*map)[boidlist->boidArr[i].xpos][boidlist->boidArr[i].ypos] == 2){
				//Move to nearest open spot simulates pushing
				findClosest(map, boidlist->boidArr[i].xpos, boidlist->boidArr[i].ypos, width, height, location);
				boidlist->boidArr[i].xpos = location[0];
				boidlist->boidArr[i].ypos = location[1];

				(*map)[boidlist->boidArr[i].xpos][boidlist->boidArr[i].ypos] = 2;

			}else if((*map)[boidlist->boidArr[i].xpos][boidlist->boidArr[i].ypos] == 0){
				printf("Error in placement\n");
			}
		}
	}
	
	return count;
}


void printBoard(short ** map, int mapwidth, int mapheight){
	int i, j;
	for(j = 0; j < mapheight; j++){
		for(i = 0; i < mapwidth; i++){
			printf("%hd", map[i][j]);
		}
		printf("\n");
	}
	printf("\n\n");
}


int main(int argc, char * argv[]){
	boidContainer container;
	goalContainer goals;
	char * fileName = NULL;
	short ** map = NULL;
	short ** blankMap = NULL;
	unsigned int mapwidth, mapheight, i, j;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &worldSize);
	MPI_Comm_rank(MPI_COMM_WORLD, &myRank);

	if(argc == 2){
		fileName = argv[1];
	}else{
		//Temporary usage to determine correctness of boid movement
		fprintf(stderr, "Usage %s <input-file>\n", argv[0]);
		exit(1);
	}

	

	//Initialize containers
	container.size = 0;
	container.alloc = 10;
	container.boidArr = (boid *) calloc(container.alloc, sizeof(boid));
	
	goals.size = 0;
	goals.alloc=10;
	goals.pos = (int **) calloc(goals.alloc, sizeof(int *));

	//Set up the map and put all the boids in a boid container
	setupSimulation(fileName, &container, &goals, &map, &blankMap, &mapwidth, &mapheight);

	for(j = 0; j < mapheight; j++){
		for(i = 0; i < mapwidth; i++){
			printf("%hd", map[i][j]);
		}
		printf("\n");
	}
		
	printf("Boid size: %d\n", (int) container.size);
	for(i = 0; i < container.size; i++){
		printf("Boid %d position %d %d\n", i, container.boidArr[i].xpos, container.boidArr[i].ypos);
	}

	printf("Goals size: %d\n", (int) goals.size);
	for(i = 0; i < goals.size; i++){
		printf("Goal %d position %d %d\n", i, goals.pos[i][0], goals.pos[i][1]);
	}

	for(i = 0; i < ITERATIONS; i++){
		if(!step(&container, &goals, &map, &blankMap, mapwidth, mapheight)){
			break;
		}
		printBoard(map, mapwidth, mapheight);
		
	}

	printf("Completed in %d steps\n", i);

	MPI_Finalize();

	return 0;
}


