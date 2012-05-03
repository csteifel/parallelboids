#include "inc.h"


/* BOID FUNCTIONS
	ALL FUNCTIONS IN THIS FILE SHOULD REENTRANT
*/



int boidInsert(boidContainer * container, boid * insert){
	//Check for need to extend container
	if(container->size == container->alloc){
		container->alloc += CONTAINEREXTEND;

		if((container->boidArr = (boid *) realloc(container->boidArr, container->alloc * sizeof(boid))) == NULL){
			fprintf(stderr, "Error extending boid container, quitting...\n");
			exit(1);
		}
	}

	container->boidArr[container->size] = *insert;
	++container->size;
	return 1;
}

int boidRemove(boidContainer * container, int index){
	return 0;
}

//Calculate the vector to move towards the exit
directionVector moveToExit(const goalContainer * const goals, const boidContainer * const boidlist, int index){
	directionVector exitVector;
	int i;
	int * res;

	exitVector.x = 0;
	exitVector.y = 0;

	for(i = 0; i < goals->size; i++){
		res = goals->pos[i];
		//We want the closest goal
		if((abs(res[0] - boidlist->boidArr[index].xpos) + abs(res[1] - boidlist->boidArr[index].ypos)) < (abs(exitVector.x) + abs(exitVector.y)) || i == 0){
			exitVector.x = res[0] - boidlist->boidArr[index].xpos;
			exitVector.y = res[1] - boidlist->boidArr[index].ypos;
		}
	}

	return exitVector;
}

//Try to keep boids away from each other so they don't colllide
directionVector aversion(const boidContainer * const boidlist, int index){
	directionVector aversionVec;
	int i;

	aversionVec.x = 0;
	aversionVec.y = 0;

	for(i = 0; i < boidlist->size; i++){
		if(i == index){
			continue;
		}
		if((abs(boidlist->boidArr[index].xpos - boidlist->boidArr[i].xpos) + abs(boidlist->boidArr[index].ypos - boidlist->boidArr[i].ypos)) < SEPARATIONWIDTH){
			aversionVec.x += boidlist->boidArr[i].xpos - boidlist->boidArr[index].xpos;
			aversionVec.y += boidlist->boidArr[i].ypos - boidlist->boidArr[index].ypos;
		}
	}

	return aversionVec;
}


//Try to move towards the center of the group
directionVector alignment(const boidContainer * const boidlist, int index){
	directionVector alignVec;
	int i, count = 0;
	alignVec.x = 0;
	alignVec.y = 0;

	for(i = 0; i < boidlist->size; i++){
		if(i == index){
			continue;
		}
		
		if((abs(boidlist->boidArr[index].xpos - boidlist->boidArr[i].xpos) + abs(boidlist->boidArr[index].ypos - boidlist->boidArr[i].ypos)) < ALIGNWIDTH){
			//If the boid we are looking at is within a certain distance then try to align with it
			alignVec.x += boidlist->boidArr[index].velocity.x;
			alignVec.y += boidlist->boidArr[index].velocity.y;
			count ++;
		}
		
	}

	//Take the average
	if(count > 0){
		alignVec.x = alignVec.x/count;
		alignVec.y = alignVec.y/count;
	}

	return alignVec;
}


//Try to steer towards the middle of the group
directionVector cohesion(const boidContainer * const boidlist, int index){
	directionVector cohesVec;
	int i, count = 0;
	
	cohesVec.x = 0;
	cohesVec.y = 0;

	for(i = 0; i < boidlist->size; i++){
		if(i == index){
			continue;
		}
		
		if((abs(boidlist->boidArr[index].xpos - boidlist->boidArr[i].xpos) + abs(boidlist->boidArr[index].ypos - boidlist->boidArr[i].ypos)) < COHEREWIDTH){
			//If the boid we are looking at is within a certain distance then try to align with it
			cohesVec.x += boidlist->boidArr[index].xpos;
			cohesVec.y += boidlist->boidArr[index].ypos;
			count++;
		}
		
		
	}

	//Take the average location of boids
	if(count > 0){
		cohesVec.x = cohesVec.x/count - boidlist->boidArr[index].xpos;
		cohesVec.y = cohesVec.y/count - boidlist->boidArr[index].ypos;
	}
	

	return cohesVec;
}


//Add vector quantities with a weight
inline void addVector(directionVector * addTo, const directionVector * const source, int weight){
	addTo->x += source->x * weight;
	addTo->y += source->y * weight;
}


//Limit boid movement to one block in one direction
inline void limitVec(directionVector * limitWhat){
	if(limitWhat->x == 0 && limitWhat->y == 0){
		//Do nothing
	}else if(abs(limitWhat->x) > abs(limitWhat->y)){
		limitWhat->y = 0;
		limitWhat->x = limitWhat->x / abs(limitWhat->x);
	}else{
		limitWhat->x = 0;
		limitWhat->y = limitWhat->y / abs(limitWhat->y);
	}
}

int inSlice (boid thisBoid, int x, int y, int X, int Y)
{
  // check if boid is active
  if (thisBoid.active == 0)
   return 0;

  // check boundaries of slice
  if (thisBoid.xpos < x)
    return 0;
  if (thisBoid.xpos >= X)
    return 0;
  if (thisBoid.ypos < y)
    return 0;
  if (thisBoid.ypos >= Y)
    return 0;

  // looks good
  return 1;
}

void moveBoid(const goalContainer * const goals, boidContainer * boidlist, boidContainer * effectingBoids, int index){
	directionVector exitVec, cohVec, alignVec, averVec, acceleration;
	int i;

	acceleration.x = 0;
	acceleration.y = 0;


	//Could possibly make these all execute in parallel as well
	exitVec = moveToExit(goals, boidlist, index);
	cohVec = cohesion(boidlist, index);
	alignVec = alignment(boidlist, index);
	averVec = aversion(boidlist, index);


	//Add up vectors with weights
	addVector(&acceleration, &exitVec, 3);
	addVector(&acceleration, &cohVec, 1);
	addVector(&acceleration, &alignVec, 1);
	addVector(&acceleration, &averVec, 2);

	//Limit the vector to required speed
	limitVec(&acceleration);

	boidlist->boidArr[index].velocity = acceleration;
	boidlist->boidArr[index].xpos += acceleration.x;
	boidlist->boidArr[index].ypos += acceleration.y;

	// check if we hit goal
	for(i = 0; i < goals->size; i++){
		if(boidlist->boidArr[index].xpos == goals->pos[i][0] && boidlist->boidArr[index].ypos == goals->pos[i][1]){
			boidlist->boidArr[index].active = 0;
		}
	}
	
}


