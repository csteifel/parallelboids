#include "inc.h"

//Boid algorithm threaded approach

//Start out with a limit on iterations until goal achievement is programmed
#define ITERATIONS

int numBoids = 0;

int main(int argc, char * argv[]){
	boidContainer container;
	char * fileName = NULL;
	short ** map = NULL;
	unsigned int mapwidth, mapheight, i, j;

	if(argc == 2){
		fileName = argv[1];
	}else{
		//Temporary usage to determine correctness of boid movement
		fprintf(stderr, "Usage %s <input-file>\n", argv[0]);
		exit(1);
	}

	//Initialize container
	container.size = 0;
	container.alloc = 10;
	container.boidArr = calloc(container.alloc, sizeof(boid));

	//Set up the map and put all the boids in a boid container
	setupSimulation(fileName, &container, &map, &mapwidth, &mapheight);

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

	return 0;
}

