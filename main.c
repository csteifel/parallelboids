#include "inc.h"

//Boid algorithm threaded approach

//Start out with a limit on iterations until goal achievement is programmed
#define ITERATIONS 10





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

int step(boidContainer * boidlist, goalContainer * goals, short *** map, short *** blankMap,
	 int width, int height)
{
        int i, count = 0;
 	int numIncoming[2];
	int indices[2];
	int outCount;
	MPI_Request recvRequest[2];
	MPI_Request sendAboveRequest[2], sendBelowRequest[2];


	boidContainer newBoids;
	boidContainer sendBelowBoids, sendAboveBoids, recvAboveBoids, recvBelowBoids;
	
	printf("STARTING RANK %d\n", rank);

       	//boidlist will contain the tasks boids already 
	// make a new array of our neighbor's boids
	newBoids.size = 0;
        newBoids.alloc = 10;
	newBoids.boidArr = (boid *) calloc(newBoids.alloc, sizeof(boid));



	//Post Irecvs for incoming information from top and bottom neighbor
	//If the top row of the map then don't recieve from the top 
	if(rank != 0){
		MPI_Irecv(&numIncoming[0], 1, MPI_INT, rank-1, 1, MPI_COMM_WORLD, &recvRequest[0]);
	}
	//If the bottom row of the map then don't recieve from the bottom
	if(rank != numranks -1 ){
		MPI_Irecv(&numIncoming[1], 1, MPI_INT, rank+1, 1, MPI_COMM_WORLD, &recvRequest[1]);
	}

	if(rank != 0){
		//Calculate which boids are within range of sending and then send number of boids and boids
		sendAboveBoids.size = 0;
		sendAboveBoids.alloc = 10;
		sendAboveBoids.boidArr = (boid *) calloc(sendAboveBoids.alloc, sizeof(boid));

		for(i = 0; i < boidlist->size; i++){
			if(boidlist->boidArr[i].ypos >= heightOffset && boidlist->boidArr[i].ypos < heightOffset + MAXWIDTH){
				boidInsert(&sendAboveBoids, &boidlist->boidArr[i]);
			}
		}


		MPI_Isend(&sendAboveBoids.size, 1, MPI_INT, rank-1, 1, MPI_COMM_WORLD, &sendAboveRequest[0]);
		MPI_Isend(&sendAboveBoids.boidArr, sendAboveBoids.size * sizeof(boid), MPI_BYTE, rank-1, 1, MPI_COMM_WORLD, &sendAboveRequest[1]);
	}

	if(rank != numranks -1){
		//Calculate which boids are within range of sending and then send number of boids and boids
		sendBelowBoids.size = 0;
		sendBelowBoids.alloc = 10;
		sendBelowBoids.boidArr = (boid *) calloc(sendBelowBoids.alloc, sizeof(boid));

		for(i = 0; i < boidlist->size; i++){
			if(boidlist->boidArr[i].ypos >= heightOffset + heightSlice - MAXWIDTH && boidlist->boidArr[i].ypos < heightOffset + heightSlice){
				boidInsert(&sendBelowBoids, &boidlist->boidArr[i]);
			}
		}


		MPI_Isend(&sendBelowBoids.size, 1, MPI_INT, rank+1, 1, MPI_COMM_WORLD, &sendBelowRequest[0]);
		MPI_Isend(&sendBelowBoids.boidArr, sendBelowBoids.size * sizeof(boid), MPI_BYTE, rank+1, 1, MPI_COMM_WORLD, &sendBelowRequest[1]);
	}

	printf("AFTER NUMBER %d\n", rank);


	if(rank != 0){
		//Recieve above boids
		while(1){
			MPI_Testsome(1, &recvRequest[0], &outCount, indices, MPI_STATUSES_IGNORE);
			if(outCount == 1 && numIncoming[0] > 0){
				recvAboveBoids.size = numIncoming[0];
				recvAboveBoids.alloc = numIncoming[0];
				recvAboveBoids.boidArr = (boid *) calloc(numIncoming[0], sizeof(boid));
				MPI_Recv(recvAboveBoids.boidArr,  sizeof(boid) * numIncoming[0],MPI_BYTE, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				break;
			}
		}
	}

	if(rank != numranks -1){
		//Recieve below boids
		while(1){
			MPI_Testsome(1, &recvRequest[1], &outCount, indices, MPI_STATUSES_IGNORE);
			if(outCount == 1 && numIncoming[1] > 0){
				recvBelowBoids.size = numIncoming[1];
				recvBelowBoids.alloc = numIncoming[1];
				recvBelowBoids.boidArr = (boid *) calloc(numIncoming[1], sizeof(boid));
				MPI_Recv(recvBelowBoids.boidArr, sizeof(boid) * numIncoming[1],MPI_BYTE, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				break;
			}
		}
	}
	


	printf("AFTER FIRST RECVS %d\n", rank);


	//New boids for now will become a container for the boids we just recieved that could possibly effect our calculations
	for(i = 0; i < recvAboveBoids.size && rank != 0; i++){
		boidInsert(&newBoids, &recvAboveBoids.boidArr[i]);
	}
	for(i = 0; i < recvBelowBoids.size && rank != numranks - 1; i++){
		boidInsert(&newBoids, &recvBelowBoids.boidArr[i]);
	}

	//Don't need array anymore already have data
	if(rank != 0)
		free(recvAboveBoids.boidArr);
	if(rank != numranks -1)
		free(recvBelowBoids.boidArr);

	printf("AFTER FREES %d\n", rank);

	for(i = 0; i < boidlist->size; i++){
		moveBoid(goals, boidlist, &newBoids, i);	
	}



	//Local boids are moved so now we have to figure out if our local boids need to change owner ship and check for incoming changes


	//Post recieves for the boids we are going to recieve from other tasks
	//If the top row of the map then don't recieve from the top 
	if(rank != 0){
		MPI_Irecv(&numIncoming[0], 1, MPI_INT, MPI_ANY_SOURCE, 2, MPI_COMM_WORLD, &recvRequest[0]);
	}
	//If the bottom row of the map then don't recieve from the bottom
	if(rank != numranks - 1){
		MPI_Irecv(&numIncoming[1], 1, MPI_INT, MPI_ANY_SOURCE, 2, MPI_COMM_WORLD, &recvRequest[1]);
	}
	
	//We are going to repurpose sendAboveBoids and sendBelowBoids here to be the one that sends boids to other processors if need be
	
	printf("POSTED SOME RECVS %d\n", rank);


	sendAboveBoids.size = 0;
	sendAboveBoids.alloc = 10;
	sendAboveBoids.boidArr = (boid *) calloc(sendAboveBoids.alloc, sizeof(boid));

	sendBelowBoids.size = 0;
	sendBelowBoids.alloc = 10;
	sendBelowBoids.boidArr = (boid *) calloc(sendBelowBoids.alloc, sizeof(boid));



	for(i = 0; i < boidlist->size; i++){
		if(boidlist->boidArr[i].ypos < heightOffset){
			//Needs to go to process above me
			boidInsert(&sendAboveBoids, &boidlist->boidArr[i]);

			//Remove boid from local list
			boidRemove(boidlist, i);
			//i--;
		}else if(boidlist->boidArr[i].ypos >= heightOffset+heightSlice){
			//Needs to go to process below me
			boidInsert(&sendBelowBoids, &boidlist->boidArr[i]);

			//Remove boid from local list
			boidRemove(boidlist, i);
			//i--;
		}
	}


	
	
	if(rank != 0){
		printf("SENDING SIZE RANK %d\n", rank);
		MPI_Isend(&sendAboveBoids.size, 1, MPI_INT, rank-1, 2, MPI_COMM_WORLD, &sendAboveRequest[0]);
		if(sendAboveBoids.size > 0)
			MPI_Isend(&sendAboveBoids.boidArr, sendAboveBoids.size * sizeof(boid), MPI_BYTE, rank-1, 2, MPI_COMM_WORLD, &sendAboveRequest[1]);
	}
	
	printf("WRONG %d\n", rank);
	if(rank != numranks -1){
		printf("SENDING SIZE RANK %d\n", rank);
		MPI_Isend(&sendBelowBoids.size, 1, MPI_INT, rank+1, 2, MPI_COMM_WORLD, &sendBelowRequest[0]);
		if(sendBelowBoids.size > 0)
			MPI_Isend(&sendBelowBoids.boidArr, sendBelowBoids.size * sizeof(boid), MPI_BYTE, rank+1, 2, MPI_COMM_WORLD, &sendBelowRequest[1]);
	}
	
	printf("WRONG2 %d\n", rank);


	//Recieve incoming boids

	if(rank != 0){
		//Recieve above boids
		while(1){
			MPI_Testsome(1, &recvRequest[0], &outCount, indices, MPI_STATUSES_IGNORE);
			if(outCount == 1){
				if(numIncoming[0] == 0){
					break;
				}
				recvAboveBoids.size = numIncoming[0];
				recvAboveBoids.alloc = numIncoming[0];
				recvAboveBoids.boidArr = (boid *) calloc(numIncoming[0], sizeof(boid));
				MPI_Recv(recvAboveBoids.boidArr, sizeof(boid) * numIncoming[0], MPI_BYTE, MPI_ANY_SOURCE, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				break;
			}
		}
	}

 	


	if(rank != numranks - 1){
		//Recieve above boids
		while(1){
			MPI_Testsome(1, &recvRequest[1], &outCount, indices, MPI_STATUSES_IGNORE);
			if(outCount == 1){
				if(numIncoming[1] > 0){
					break;
				}
				recvBelowBoids.size = numIncoming[1];
				recvBelowBoids.alloc = numIncoming[1];
				recvBelowBoids.boidArr = (boid *) calloc(numIncoming[1], sizeof(boid));
				MPI_Recv(recvBelowBoids.boidArr, sizeof(boid) * numIncoming[1], MPI_BYTE, MPI_ANY_SOURCE, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				break;
			}
		}
	}

	printf("RECIEVed INCOMING\n");

	//Add them to the local boid list
	for(i = 0; i < recvAboveBoids.size && rank != 0; i++){
		boidInsert(boidlist, &recvAboveBoids.boidArr[i]);
	}
	for(i = 0; i < recvBelowBoids.size && rank != numranks -1 ; i++){
		boidInsert(boidlist, &recvBelowBoids.boidArr[i]);
	}





	//We need to reconcile the map now based on boid positions and check for people on top of each other
	for(i = 0; i < boidlist->size; i++)
	  {
	    int location[2];
	    
	    // if boid is active and in our slice
	    //if(boidlist->boidArr[i].active == 1)
	    //printf ("%d, %d\n", boidlist->boidArr[i].xpos, boidlist->boidArr[i].ypos);
	    if(inSlice(boidlist->boidArr[i], widthOffset, heightOffset, widthOffset + widthSlice, heightOffset + heightSlice) == 1)
	      {
		// if map is free space
		if((*map)[boidlist->boidArr[i].xpos][boidlist->boidArr[i].ypos] == 1)
		  {
		    // set to boid
		    (*map)[boidlist->boidArr[i].xpos][boidlist->boidArr[i].ypos] = 2;
		  }
		// if map is occupied
		else if((*map)[boidlist->boidArr[i].xpos][boidlist->boidArr[i].ypos] == 2)
		  {
		    //Move to nearest open spot, simulates pushing
		    findClosest(map, boidlist->boidArr[i].xpos, boidlist->boidArr[i].ypos, width, height, location);
		    boidlist->boidArr[i].xpos = location[0];
		    boidlist->boidArr[i].ypos = location[1];
		    
		    (*map)[boidlist->boidArr[i].xpos][boidlist->boidArr[i].ypos] = 2;
		    
		  }
		// if map is wall
		else if((*map)[boidlist->boidArr[i].xpos][boidlist->boidArr[i].ypos] == 0)
		  {
		    // i am error
		    if (rank == 0)
		      printf("Error in placement\n");
		  }
	      }
	  }
	
	// return/repeat
	MPI_Barrier(MPI_COMM_WORLD);	
	return 0;
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
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &numranks);


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

	/*
	for(j = 0; j < mapheight; j++){
		for(i = 0; i < mapwidth; i++){
			printf("%hd", map[i][j]);
		}
		printf("\n");
	}
	*/


	
	// only let rank 0 output init stuff
	if (rank == 0 && 0)
	  {
	    printBoard(map, mapwidth, mapheight);

	    printf("Num ranks: %d\n", numranks);

	    printf("Boid size: %d\n", (int) container.size);
	    for(i = 0; i < container.size; i++){
	      printf("Boid %d position %d %d\n", i, container.boidArr[i].xpos, container.boidArr[i].ypos);
	    }
	    
	    printf("Goals size: %d\n", (int) goals.size);
	    for(i = 0; i < goals.size; i++){
	      printf("Goal %d position %d %d\n", i, goals.pos[i][0], goals.pos[i][1]);
	    }
	    
	    printf("\n");
	  }
	
	step(&container, &goals, &map, &blankMap, mapwidth, mapheight);

	// so now loop through the number of iterations
	/*for(i = 0; i < ITERATIONS; i++){
		printf("RANK %d\n", rank);
		if(!step(&container, &goals, &map, &blankMap, mapwidth, mapheight, rank, numranks)){
			break;
		}
	}*/
	if (rank == 0)
	  printf("Completed in %d steps\n", i);
	
	MPI_Finalize();
	return 0;
}


