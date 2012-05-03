#include "inc.h"

//Boid algorithm threaded approach

//Start out with a limit on iterations until goal achievement is programmed
#define ITERATIONS 10

struct arguments {
	boidContainer * boidlist;
	goalContainer * goals;
	int start;
	int finish;
};



//Find the closest open spot on the map and place the boid there
void findClosest(short *** map, int x, int y, int width, int height, int * positions){
	int lookRadius = 1;
	int i, j, placed = 0;

	printf("doing findClosest\n");
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
	 int width, int height, int widthSlice, int heightSlice,
	 int widthOffset, int heightOffset, int rank, int numranks)
{
        int i, count = 0;
        
        // get the array of just our boids
        boidContainer ourBoids;
        ourBoids.size = 0;
        ourBoids.alloc = 10;
	ourBoids.boidArr = (boid *) calloc(ourBoids.alloc, sizeof(boid));

	// iterate through the entire boidlist
	for(i = 0; i < boidlist->size; i++)
	  {
	    // check if the boid is in our slice
	    if(inSlice(boidlist->boidArr[i], widthOffset, heightOffset, widthOffset + widthSlice, heightOffset + heightSlice) == 1)
	      {
		// also move this boid
		moveBoid(goals, boidlist, i);
		// add to new container if so
		boidInsert(&ourBoids, &boidlist->boidArr[i]);
	      }
	  }

	// make a new array of our neighbor's boids
	boidContainer newBoids;
	newBoids.size = 0;
        newBoids.alloc = 10;
	newBoids.boidArr = (boid *) calloc(newBoids.alloc, sizeof(boid));

	// send all of our boids to all of our neighbors
	
	int numIncoming = 0;
	int source = 0;
	int dest = 0;
	int tag = 0;
	MPI_Request recvRequest[1];
	int indices[1];

	// irecv number of boids incoming	
	// tag for first send/recv operation
	tag = 1;
	// receive from rank below us first (above in the map)
	source = rank - 1;
	// rank 0 does not have a rank to receive from right now
	if (source != -1)
	  MPI_Irecv(&numIncoming, 1, MPI_INT, source, tag, MPI_COMM_WORLD, &recvRequest[0]);
	// now do sends to rank above us (below in map)
	dest = rank + 1;
	// rank numranks - 1 has no one to send to
	if (dest != numranks)
	  {
	    //printf("%d: sending %d boids to %d\n", rank, ourBoids.size, dest);
	    //printf("%d: pre send 1\n", rank);
	    MPI_Send(&ourBoids.size, 1, MPI_INT, dest, tag, MPI_COMM_WORLD);
	    //printf("%d: post send 1\n", rank);
	  }

	// wait for receives
	if (source != -1)
	  {
	    count = 0;
	    //printf("%d: pre testsome 1\n", rank);
	    while (count <= 0)
	      MPI_Testsome(1, recvRequest, &count, indices, MPI_STATUSES_IGNORE);
	    //printf("%d: post testsome 1\n", rank);
	  }

	int *boidData;

	// tag for sending/receiving the boids
	tag = 2;

	// if nothing incoming don't bother with this step
	if (numIncoming > 0)
	  {
	    // allocate space for the data we will receive
	    boidData = (int *) calloc(numIncoming*6, sizeof(int));

	    MPI_Irecv(&boidData[0], numIncoming*6, MPI_INT, source, tag, MPI_COMM_WORLD, &recvRequest[0]);
	  }

	// if nothing outgoing don't do this step
	if (ourBoids.size > 0 && dest != numranks)
	  {
	    // allocate space for the data we will send
	    boidData = (int *) calloc(ourBoids.size*6, sizeof(int));

	    // populate the data we will send
	    for (i = 0; i < ourBoids.size; i++)
	      {
		boidData[6*i+0] = ourBoids.boidArr[i].xpos;
		boidData[6*i+1] = ourBoids.boidArr[i].ypos;
		boidData[6*i+2] = ourBoids.boidArr[i].active;
		boidData[6*i+3] = ourBoids.boidArr[i].velocity.x;
		boidData[6*i+4] = ourBoids.boidArr[i].velocity.y;
		boidData[6*i+5] = ourBoids.boidArr[i].id;
	      }
	    //printf("%d: sending %d boids to %d\n", rank, ourBoids.size, dest);
	    //printf("%d: pre send 2\n", rank);
	    MPI_Send(&boidData[0], ourBoids.size*6, MPI_INT, dest, tag, MPI_COMM_WORLD);
	    //printf("%d: post send 2\n", rank);
	  }

	// wait for receives
	if (source != -1 && numIncoming > 0)
	  {
	    count = 0;
	    //printf("%d: pre testsome 2\n", rank);
	    while (count <= 0)
	      MPI_Testsome(1, recvRequest, &count, indices, MPI_STATUSES_IGNORE);
	    //printf("%d: post testsome 2\n", rank);
	  }

	boid * newBoid = NULL;

	// if we were expecting data
	if (numIncoming > 0)
	  {
	    for (i = 0; i < numIncoming; i++)
	      {
		newBoid = (boid *) calloc(1, sizeof(boid));
		newBoid->xpos = boidData[6*i+0];
		newBoid->ypos = boidData[6*i+1];
		newBoid->active = boidData[6*i+2];
		newBoid->velocity.x = boidData[6*i+3];
		newBoid->velocity.y = boidData[6*i+4];
		newBoid->id = boidData[6*i+5];
		boidInsert(&newBoids, newBoid);
		newBoid = NULL;
	      }
	  }

	// repeat for other direction
	numIncoming = 0;

	// irecv number of boids incoming
	// tag for first send/recv operation (second time)
	tag = 3;
	// receive from rank above us first (below in the map)
	source = rank + 1;
	// rank 0 does not have a rank to receive from right now
	if (source != numranks)
	  MPI_Irecv(&numIncoming, 1, MPI_INT, source, tag, MPI_COMM_WORLD, &recvRequest[0]);
	// now do sends to rank below us (above in map)
	dest = rank - 1;
	// rank 0 has no one to send to
	if (dest != -1)
	  {
	    //printf("%d: sending %d boids to %d\n", rank, ourBoids.size, dest);
	    //printf("%d: pre send 3\n", rank);
	    MPI_Send(&ourBoids.size, 1, MPI_INT, dest, tag, MPI_COMM_WORLD);
	    //printf("%d: post send 3\n", rank);
	  }

	// wait for receives
	if (source != numranks)
	  {
	    count = 0;
	    //printf("%d: pre testsome 3\n", rank);
	    while (count <= 0)
	      MPI_Testsome(1, recvRequest, &count, indices, MPI_STATUSES_IGNORE);
	    //printf("%d: post testsome 3\n", rank);
	  }

	// tag for sending/receiving the boids (second time)
	tag = 4;

	// if nothing incoming don't bother with this step
	if (numIncoming > 0)
	  {
	    // allocate space for the data we will receive
	    boidData = (int *) calloc(numIncoming*6, sizeof(int));

	    MPI_Irecv(&boidData[0], numIncoming*6, MPI_INT, source, tag, MPI_COMM_WORLD, &recvRequest[0]);
	  }
	// if nothing outgoing don't do this step
	if (ourBoids.size > 0 && dest != -1)
	  {
	    // allocate space for the data we will send
	    boidData = (int *) calloc(ourBoids.size*6, sizeof(int));

	    // populate the data we will send
	    for (i = 0; i < ourBoids.size; i++)
	      {
		boidData[6*i+0] = ourBoids.boidArr[i].xpos;
		boidData[6*i+1] = ourBoids.boidArr[i].ypos;
		boidData[6*i+2] = ourBoids.boidArr[i].active;
		boidData[6*i+3] = ourBoids.boidArr[i].velocity.x;
		boidData[6*i+4] = ourBoids.boidArr[i].velocity.y;
		boidData[6*i+5] = ourBoids.boidArr[i].id;
	      }
	    //printf("%d: sending %d boids to %d\n", rank, ourBoids.size, dest);
	    //printf("%d: pre send 4\n", rank);
	    MPI_Send(&boidData[0], ourBoids.size*6, MPI_INT, dest, tag, MPI_COMM_WORLD);
	    //printf("%d: post send 4\n", rank);
	  }

	// wait for receives
	if (source != numranks && numIncoming > 0)
	  {
	    count = 0;
	    //printf("%d: pre testsome 4\n", rank);
	    while (count <= 0)
	      MPI_Testsome(1, recvRequest, &count, indices, MPI_STATUSES_IGNORE);
	    //printf("%d: post testsome 4\n", rank);
	  }

	// if we were expecting data
	if (numIncoming > 0)
	  {
	    for (i = 0; i < numIncoming; i++)
	      {
		newBoid = (boid *) calloc(1, sizeof(boid));
		newBoid->xpos = boidData[6*i+0];
		newBoid->ypos = boidData[6*i+1];
		newBoid->active = boidData[6*i+2];
		newBoid->velocity.x = boidData[6*i+3];
		newBoid->velocity.y = boidData[6*i+4];
		newBoid->id = boidData[6*i+5];
		boidInsert(&newBoids, newBoid);
		newBoid = NULL;
	      }
	  }

	// done sending and receiving boids to and from neighbors

	// free map and copy blankMap over
	for(i = 0; i < width; i++){
		free((*map)[i]);
		(*map)[i] = (short *) calloc(height, sizeof(short));
		memcpy((*map)[i], (*blankMap)[i], height * sizeof(short));
	}

	// we do not care about boidlist any more, we just care about ourBoids
	// + newBoids
	boidlist->size = 0;
	boidlist->alloc = ourBoids.size + newBoids.size;
	boidlist->boidArr = (boid *) calloc(boidlist->alloc, sizeof(boid));

	for (i = 0; i < ourBoids.size; i++)
	  {
	    boidInsert(boidlist, &ourBoids.boidArr[i]);
	  }
	for (i = 0; i < newBoids.size; i++)
	  {
	    boidInsert(boidlist, &newBoids.boidArr[i]);
	  }

	//if (boidlist->size != 0)
	//printf("size: %d\n", boidlist->size);

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
	
	// barrier
	MPI_Barrier(MPI_COMM_WORLD);
	
	// return/repeat
	
	return 1;
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

	// mpi stuff
	int rank, numranks;
	int widthOffset, heightOffset;
	int widthSlice, heightSlice;

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

	// okay every rank gets the entire map w/ boids

	// now calculate our slice
	widthSlice = mapwidth;
	heightSlice = mapheight / numranks;
	widthOffset = 0;
	heightOffset = heightSlice * rank;
	// so we only work with boids in the range
	// (widthOffset, heightOffset) to (widthOffset + widthSlice, heightOffset + heightSlice)
	//printf("%d, %d, %d, %d\n", widthOffset, heightOffset, widthSlice, heightSlice);
	
	// only let rank 0 output init stuff
	if (rank == 0)
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
	
	// so now loop through the number of iterations
	for(i = 0; i < ITERATIONS; i++)
	  {
	    for (j = 0; j < container.size; j ++)
	      {
		/*
		if(inSlice(container.boidArr[j], widthOffset, heightOffset,
			   widthOffset + widthSlice, heightOffset + heightSlice) == 1)	      
		printf("before iteration: %d rank %d: boid: %d: %d, %d\n", i, rank, container.boidArr[j].id,
			 container.boidArr[j].xpos, container.boidArr[j].ypos);
		*/
	      }

	    if(!step(&container, &goals, &map, &blankMap, mapwidth, mapheight,
		     widthSlice, heightSlice, widthOffset, heightOffset, rank, numranks))
	      {
		break;
	      }

	    if (rank == 2)
	      {
		//printf("iteration: %d\n", i);
		//printBoard(map, mapwidth, mapheight);
	      }

	    
	    for (j = 0; j < container.size; j ++)
	      {
		
		//if(inSlice(container.boidArr[j], widthOffset, heightOffset,
		//	   widthOffset + widthSlice, heightOffset + heightSlice) == 1)	      
		//printf("after  iteration: %d rank %d: boid: %d: %d, %d\n", i, rank, container.boidArr[j].id,
		//	 container.boidArr[j].xpos, container.boidArr[j].ypos);
		
	      }
	  }
	
	if (rank == 0)
	  printf("Completed in %d steps\n", i);
	
	MPI_Finalize();
	return 0;
}


