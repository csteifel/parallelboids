#ifndef boid_h
#define boid_h


#define COHESION_WEIGHT
#define ALIGNMENT_WEIGHT
#define AVERSION_WEIGHT
#define COLLISION_WEIGHT 
#define EXIT_WEIGHT


//Used for velocity and acceleration
struct directionVector {
	int x;
	int y;
};

typedef struct directionVector directionVector;


struct boid {
	int xpos;
	int ypos;
	directionVector velocity;
	int active;
};

typedef struct boid boid;


struct boidContainer {
	boid * boidArr;
	size_t size;
	size_t alloc;
};

typedef struct boidContainer boidContainer;

//List prototypes for boid acceleration calculation
//All should return directional vectors for acceleration calculation
directionVector cohesion(boid * boidArr, int index);
directionVector aversion(boid * boidArr, int index);
directionVector collision(boid * boidArr, int index);
directionVector alignment(boid * boidArr, int index);
directionVector moveToExit(boid * boidArr, int index);

//boidContainer functions
void boidInsert(boid insert);

#endif
