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
};



typedef struct boid boid;


struct directionVector cohesion(boid * boidArr, int index);
struct directionVector aversion(boid * boidArr, int index);
struct directionVector collision(boid * boidArr, int index);
struct directionVector alignment(boid * boidArr, int index);
struct directionVector exit(boid * boidArr, int index);

#endif
