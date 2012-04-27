#include "inc.h"

//Boid algorithm threaded approach

//Start out with a limit on iterations until goal achievement is programmed
#define ITERATIONS

int numBoids = 0;

int main(int argc, char * argv[]){
	boid * boidsContainer;


	if(argc == 2){
		
	}else{
		//Temporary usage to determine correctness of boid movement
		fprintf(stderr, "Usage %s <num-boids>\n", argv[0]);
		exit(1);
	}
	
	boidContainer boids;


	return 0;
}

