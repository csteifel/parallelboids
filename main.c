#include "inc.h"

//Boid algorithm threaded approach

//Start out with a limit on iterations until goal achievement is programmed
#define ITERATIONS 9000


unsigned long long procSpeed = 700000000;

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
        int i, count = 0, total = 0;
 	int numIncoming[2];
	int indices[2];
	int outCount;
	int needed;
	int soFar;
	MPI_Request recvRequest[2];
	MPI_Request sendAboveRequest[2], sendBelowRequest[2];


	boidContainer newBoids;
	boidContainer sendBelowBoids, sendAboveBoids, recvAboveBoids, recvBelowBoids;
	
       	//boidlist will contain the tasks boids already 
	// make a new array of our neighbor's boids
	newBoids.size = 0;
        newBoids.alloc = 10;
	newBoids.boidArr = (boid *) calloc(newBoids.alloc, sizeof(boid));

	//Post Irecvs for incoming information from top and bottom neighbor
	//If the top row of the map then don't recieve from the top 
	if(rank != 0){
		recvTime = rdtsc();
		MPI_Irecv(&numIncoming[0], 1, MPI_INT, rank-1, 1, MPI_COMM_WORLD, &recvRequest[0]);
		recvAmount += sizeof(int);
		recvTimeTotal += rdtsc() - recvTime;
	}
	//If the bottom row of the map then don't recieve from the bottom
	if(rank != numranks -1 ){
		recvTime = rdtsc();
		MPI_Irecv(&numIncoming[1], 1, MPI_INT, rank+1, 1, MPI_COMM_WORLD, &recvRequest[1]);
		recvAmount += sizeof(int);
		recvTimeTotal += rdtsc() - recvTime;
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
		

		sendTime = rdtsc();
		MPI_Isend(&sendAboveBoids.size, 1, MPI_INT, rank-1, 1, MPI_COMM_WORLD, &sendAboveRequest[0]);
		sendAmount += sizeof(int);
		if(sendAboveBoids.size > 0){
			MPI_Isend(sendAboveBoids.boidArr, sendAboveBoids.size * sizeof(boid), MPI_BYTE, rank-1, 2, MPI_COMM_WORLD, &sendAboveRequest[1]);
			sendAmount += sendAboveBoids.size * sizeof(boid);
		}
		sendTimeTotal += rdtsc() - sendTime;
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


		sendTime = rdtsc();
		MPI_Isend(&sendBelowBoids.size, 1, MPI_INT, rank+1, 1, MPI_COMM_WORLD, &sendBelowRequest[0]);
		sendAmount += sizeof(int);
		if(sendBelowBoids.size > 0){
			MPI_Isend(sendBelowBoids.boidArr, sendBelowBoids.size * sizeof(boid), MPI_BYTE, rank+1, 2, MPI_COMM_WORLD, &sendBelowRequest[1]);
			sendAmount += sendBelowBoids.size * sizeof(boid);
		}
		sendTimeTotal += rdtsc() - sendTime;
	}

	if(rank != 0){
		recvTime = rdtsc();
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
				MPI_Recv(recvAboveBoids.boidArr,  sizeof(boid) * numIncoming[0],MPI_BYTE, rank-1, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				recvAmount += sizeof(boid) * numIncoming[0];
				break;
			}
		}
		recvTimeTotal += rdtsc() - recvTime;
	}else{
		numIncoming[0] = 0;
	}
	
	if(rank != numranks -1){
		recvTime = rdtsc();
		//Recieve below boids
		while(1){
			MPI_Testsome(1, &recvRequest[1], &outCount, indices, MPI_STATUSES_IGNORE);	
			if(outCount == 1){
				if(numIncoming[1] == 0){
					break;
				}
				recvBelowBoids.size = numIncoming[1];
				recvBelowBoids.alloc = numIncoming[1];
				recvBelowBoids.boidArr = (boid *) calloc(numIncoming[1], sizeof(boid));
				MPI_Recv(recvBelowBoids.boidArr, sizeof(boid) * numIncoming[1],MPI_BYTE, rank+1, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				recvAmount += sizeof(boid) * numIncoming[1];
				break;
			}
		}
		recvTimeTotal += rdtsc() - recvTime;
	}else{
		numIncoming[1] = 0;
	}


	//New boids for now will become a container for the boids we just recieved that could possibly effect our calculations
	for(i = 0; i < recvAboveBoids.size && rank != 0 && numIncoming[0] > 0; i++){
		boidInsert(&newBoids, &recvAboveBoids.boidArr[i]);
	}
	for(i = 0; i < recvBelowBoids.size && rank != numranks - 1 && numIncoming[1] > 0; i++){
		boidInsert(&newBoids, &recvBelowBoids.boidArr[i]);
	}


	//Don't need array anymore already have data
	if(rank != 0 && numIncoming[0] > 0)
		free(recvAboveBoids.boidArr);
	if(rank != numranks -1 && numIncoming[1] > 0)
		free(recvBelowBoids.boidArr);



	//Make sure the values have been sent
	if(rank != 0){
		sendTime = rdtsc();
		soFar = 0;
		while(1){
			if(sendAboveBoids.size > 0){
				needed = 2;
			}else{
				needed = 1;
			}
			MPI_Testsome(needed, sendAboveRequest, &outCount, indices, MPI_STATUSES_IGNORE);
			soFar += outCount;
			if(soFar == needed){
				break;
			}
		}
		sendTimeTotal += rdtsc() - sendTime;
		free(sendAboveBoids.boidArr);
	}
	
	if(rank != numranks -1){
		sendTime = rdtsc();
		soFar = 0;
		while(1){
			if(sendBelowBoids.size > 0){
				needed = 2;
			}else{
				needed = 1;
			}
			MPI_Testsome(needed, sendBelowRequest, &outCount, indices, MPI_STATUSES_IGNORE);
			soFar += outCount;
			if(soFar == needed){
				break;
			}
		}
		sendTimeTotal += rdtsc() - sendTime;
		free(sendBelowBoids.boidArr);
	}


	for(i = 0; i < boidlist->size; i++){
		moveBoid(goals, boidlist, &newBoids, i);	
	}

	free(newBoids.boidArr);

	//Local boids are moved so now we have to figure out if our local boids need to change owner ship and check for incoming changes


	//Post recieves for the boids we are going to recieve from other tasks
	//If the top row of the map then don't recieve from the top 
	if(rank != 0){
		recvTime = rdtsc();
		MPI_Irecv(&numIncoming[0], 1, MPI_INT, rank-1, 3, MPI_COMM_WORLD, &recvRequest[0]);
		recvAmount += sizeof(int);
		recvTimeTotal += rdtsc() - recvTime;
	}
	//If the bottom row of the map then don't recieve from the bottom
	if(rank != numranks - 1){
		recvTime = rdtsc();
		MPI_Irecv(&numIncoming[1], 1, MPI_INT, rank+1, 3, MPI_COMM_WORLD, &recvRequest[1]);
		recvAmount += sizeof(int);
		recvTimeTotal += rdtsc() - recvTime;
	}
	
	//We are going to repurpose sendAboveBoids and sendBelowBoids here to be the one that sends boids to other processors if need be
	

	sendAboveBoids.size = 0;
	sendAboveBoids.alloc = 10;
	sendAboveBoids.boidArr = (boid *) calloc(sendAboveBoids.alloc, sizeof(boid));

	sendBelowBoids.size = 0;
	sendBelowBoids.alloc = 10;
	sendBelowBoids.boidArr = (boid *) calloc(sendBelowBoids.alloc, sizeof(boid));

	//printf("%d %d\n", heightOffset, heightSlice);
	for(i = 0; i < boidlist->size; i++){
		//if(rank == 1)
			//printf("Boid pos: %d %d %d %d %d\n", i, boidlist->boidArr[i].xpos, boidlist->boidArr[i].ypos, rank, boidlist->boidArr[i].ypos  >= heightOffset+heightSlice);
		if(boidlist->boidArr[i].ypos < heightOffset){
			//printf("GOING\n");
			//Needs to go to process above me
			boidInsert(&sendAboveBoids, &boidlist->boidArr[i]);

			//Remove boid from local list
			boidRemove(boidlist, i);
			i--;
			continue;
		}
		if(boidlist->boidArr[i].ypos >= heightOffset+heightSlice){
			//Needs to go to process below me
			boidInsert(&sendBelowBoids, &boidlist->boidArr[i]);

			//Remove boid from local list
			boidRemove(boidlist, i);
			i--;
			continue;
		}
	}


	//Send size information and boids to other process
	
	if(rank != 0){
		sendTime = rdtsc();
		MPI_Isend(&sendAboveBoids.size, 1, MPI_INT, rank-1, 3, MPI_COMM_WORLD, &sendAboveRequest[0]);
		sendAmount += sizeof(int);
		if(sendAboveBoids.size > 0){
			MPI_Isend(sendAboveBoids.boidArr, sendAboveBoids.size * sizeof(boid), MPI_BYTE, rank-1, 4, MPI_COMM_WORLD, &sendAboveRequest[1]);
			sendAmount += sendAboveBoids.size * sizeof(boid);
		}
		sendTimeTotal += rdtsc() - sendTime;
	}
	
	if(rank != numranks -1){
		sendTime = rdtsc();
		MPI_Isend(&sendBelowBoids.size, 1, MPI_INT, rank+1, 3, MPI_COMM_WORLD, &sendBelowRequest[0]);
		sendAmount += sizeof(int);
		if(sendBelowBoids.size > 0){
			MPI_Isend(sendBelowBoids.boidArr, sendBelowBoids.size * sizeof(boid), MPI_BYTE, rank+1, 4, MPI_COMM_WORLD, &sendBelowRequest[1]);
			sendAmount += sendBelowBoids.size * sizeof(boid);
		}
		sendTimeTotal += rdtsc() - sendTime;
	}
	


	//Make sure the values have been sent
	if(rank != 0){
		sendTime = rdtsc();
		soFar = 0;
		while(1){
			if(sendAboveBoids.size > 0){
				needed = 2;
			}else{
				needed = 1;
			}
			MPI_Testsome(needed, sendAboveRequest, &outCount, indices, MPI_STATUSES_IGNORE);
			soFar += outCount;
			if(soFar == needed){
				break;
			}
		}
		sendTimeTotal += rdtsc() - sendTime;
		free(sendAboveBoids.boidArr);
	}
	
	if(rank != numranks -1){
		sendTime = rdtsc();
		soFar = 0;
		while(1){
			if(sendBelowBoids.size > 0){
				needed = 2;
			}else{
				needed = 1;
			}
			MPI_Testsome(needed, sendBelowRequest, &outCount, indices, MPI_STATUSES_IGNORE);
			soFar += outCount;
			if(soFar == needed){
				break;
			}
		}
		sendTimeTotal += rdtsc() - sendTime;
		free(sendBelowBoids.boidArr);
	}

	//Recieve incoming boids

	if(rank != 0){
		//Recieve above boids
		recvTime = rdtsc();
		while(1){
			MPI_Testsome(1, &recvRequest[0], &outCount, indices, MPI_STATUSES_IGNORE);
			if(outCount == 1){
				if(numIncoming[0] == 0){
					recvAboveBoids.size = 0;
					break;
				}
				recvAboveBoids.size = numIncoming[0];
				recvAboveBoids.alloc = numIncoming[0];
				recvAboveBoids.boidArr = (boid *) calloc(numIncoming[0], sizeof(boid));
				MPI_Recv(recvAboveBoids.boidArr, sizeof(boid) * numIncoming[0], MPI_BYTE, rank-1, 4, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				recvAmount += sizeof(boid) * numIncoming[0];
				break;
			}
		}
		recvTimeTotal += rdtsc() - recvTime;
	}else{
		numIncoming[0] = 0;
	}


	if(rank != numranks - 1){
		recvTime = rdtsc();
		//Recieve below boids
		while(1){
			MPI_Testsome(1, &recvRequest[1], &outCount, indices, MPI_STATUSES_IGNORE);
			if(outCount == 1){
				if(numIncoming[1] == 0){
					recvBelowBoids.size = 0;
					break;
				}
				recvBelowBoids.size = numIncoming[1];
				recvBelowBoids.alloc = numIncoming[1];
				recvBelowBoids.boidArr = (boid *) calloc(numIncoming[1], sizeof(boid));
				MPI_Recv(recvBelowBoids.boidArr, sizeof(boid) * numIncoming[1], MPI_BYTE, rank+1, 4, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				recvAmount += sizeof(boid) * numIncoming[1];
				break;
			}
		}
		recvTimeTotal += rdtsc() - recvTime;
	}else{
		numIncoming[1] = 0;
	}



	//Add them to the local boid list
	for(i = 0; i < recvAboveBoids.size && rank != 0; i++){
		boidInsert(boidlist, &recvAboveBoids.boidArr[i]);
	}


	for(i = 0; i < recvBelowBoids.size && rank != numranks -1 ; i++){
		boidInsert(boidlist, &recvBelowBoids.boidArr[i]);
	}

	for(i = 0; i < boidlist->size; i++){
		if(boidlist->boidArr[i].active == 1){
			count++;
		}else if(boidlist->boidArr[i].active == 0){
			//No longer need to track the boid
			boidRemove(boidlist, i);
			i--;
			continue;
		}
	}
	
	
	if(rank != 0 && recvAboveBoids.size > 0){
		free(recvAboveBoids.boidArr);	
	}
	if(rank != numranks - 1 && recvBelowBoids.size > 0){
		free(recvBelowBoids.boidArr);
	}

	for(i = 0; i < width; i++){
		free((*map)[i]);
		(*map)[i] = (short *) calloc(height, sizeof(short));
		memcpy((*map)[i], (*blankMap)[i], height * sizeof(short));
	}

	//We need to reconcile the map now based on boid positions and check for people on top of each other
	for(i = 0; i < boidlist->size; i++)
	  {
	    int location[2];
	 	
		if(rank == 0){
			//printf("%d\n", boidlist->boidArr[i].xpos);
			//printf("Itr %d\n", i);
		}
	    // if boid is active and in our slice
	    //if(boidlist->boidArr[i].active == 1)
	    //printf ("%d, %d\n", boidlist->boidArr[i].xpos, boidlist->boidArr[i].ypos);
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
		    
		  }else if((*map)[boidlist->boidArr[i].xpos][boidlist->boidArr[i].ypos] == 3){
				boidlist->boidArr[i].active = 0;
		  }
		  // if map is wall
		else if((*map)[boidlist->boidArr[i].xpos][boidlist->boidArr[i].ypos] == 0)
		  {
		    // i am error
		    if (rank == 0)
		      printf("Error in placement\n");
		  }

		//printf("%d %d %d\n",i, boidlist->boidArr[i].xpos, boidlist->boidArr[i].ypos);


	      }
	  
	  //printf("RECONCILED %d\n", rank);
	
	// return/repeat
	MPI_Allreduce(&count, &total, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
	//printf("Count %d total %d\n", count, total);
	return total;
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
	unsigned int mapwidth, mapheight, i;
	int counter;
	sendTimeTotal = 0;
	recvTimeTotal = 0;
	sendAmount = 0;
	recvAmount = 0;

	execTime = rdtsc();
	
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
	setupTime = rdtsc();
	setupSimulation(fileName, &container, &goals, &map, &blankMap, &mapwidth, &mapheight);
	setupTime = rdtsc() - setupTime;
	if(rank == 0)
		fprintf(stderr, "SET UP DONE\n");
	if(rank == 0)
		printf("Setup time: %lf\n", (double) setupTime / procSpeed);
	
	// only let rank 0 output init stuff
	/*if (rank == 0 && 0)
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
	*/	
	// so now loop through the number of iterations
	for(i = 0; i < ITERATIONS; i++){
		stepTime = rdtsc();
		counter = step(&container, &goals, &map, &blankMap, mapwidth, mapheight);
		stepTime = rdtsc() - stepTime;
		MPI_Allreduce(&stepTime, &stepTotal, 1, MPI_LONG_LONG, MPI_SUM, MPI_COMM_WORLD);
		if(rank == 0)
			printf("Step time average: %d %lf\n", i, (double) stepTotal/numranks / procSpeed);
		if(counter == 0){
			break;
		}
	}
	if (rank == 0)
	  printf("Completed in %d steps\n", i);
	
	
	execTime = rdtsc() - execTime;
	if(rank == 0)
		printf("Exec time: %lf\n", (double) execTime / procSpeed);
	
	MPI_Allreduce(&sendTime, &sendTimeTotal, 1, MPI_LONG_LONG, MPI_SUM, MPI_COMM_WORLD);
	MPI_Allreduce(&sendTime, &sendTimeTotal, 1, MPI_LONG_LONG, MPI_SUM, MPI_COMM_WORLD);


	MPI_Allreduce(&sendAmount, &sendAmountTotal, 1, MPI_LONG_LONG, MPI_SUM, MPI_COMM_WORLD);
	MPI_Allreduce(&recvAmount, &recvAmountTotal, 1, MPI_LONG_LONG, MPI_SUM, MPI_COMM_WORLD);
	
	if(rank == 0){
		printf("Total sent %llu average time %lf\n", sendAmountTotal, (double) sendTimeTotal/numranks / procSpeed);
		printf("Total receive %llu average time %lf\n", recvAmountTotal, (double) recvTimeTotal/numranks / procSpeed);
	}

	MPI_Finalize();
	

	return 0;
}


