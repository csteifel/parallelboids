#include "inc.h"


int boidInsert(boidContainer * container, boid * insert){
	//Check for need to extend container
	if(container->size == container->alloc){
		container->alloc += CONTAINEREXTEND;
	}
	if(realloc(container, container->alloc * sizeof(boid)) == NULL){
		fprintf(stderr, "Error extending boid container, quitting...\n");
		exit(1);
	}
	
	container->boidArr[container->size] = *insert;
	return 1;
}
