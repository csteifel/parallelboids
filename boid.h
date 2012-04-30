#ifndef boid_h
#define boid_h

#define COHESION_WEIGHT 1
#define ALIGNMENT_WEIGHT 1
#define AVERSION_WEIGHT 1
#define COLLISION_WEIGHT 1
#define EXIT_WEIGHT 1



#define ALIGNWIDTH 4
#define COHEREWIDTH 4

//Used for velocity and acceleration
struct directionVector {
	//X and Y should be delta position not absolute position - i.e. move to the left should be x < 0
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
void moveBoid(const goalContainer * const goals, boidContainer * boidlist, int index);
directionVector cohesion(const boidContainer * const boidlist, int index);
directionVector aversion(const boidContainer * const boidlist, int index);
directionVector collision(const boidContainer * const boidlist, int index);
directionVector alignment(const boidContainer * const boidlist, int index);
directionVector moveToExit(const goalContainer * const goals, const boidContainer * const boidlist, int index);

directionVector getGoalDistance(int goalx, int goaly, const boid * const b);

#define CONTAINEREXTEND 10

//boidContainer functions
int boidInsert(boidContainer * container, boid * insert);

#endif
