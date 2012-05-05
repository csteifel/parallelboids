#ifndef boid_h
#define boid_h

#define COHESION_WEIGHT 1
#define ALIGNMENT_WEIGHT 1
#define AVERSION_WEIGHT 1
#define COLLISION_WEIGHT 1
#define EXIT_WEIGHT 1



#define ALIGNWIDTH 4
#define COHEREWIDTH 4
#define SEPARATIONWIDTH 2
#define MAXWIDTH MAX(MAX(ALIGNWIDTH, COHEREWIDTH), SEPARATIONWIDTH)

//Used for velocity and acceleration
struct directionVector {
	//X and Y should be delta position not absolute position - i.e. move to the left should be x < 0
	int x;
	int y;
};

typedef struct directionVector directionVector;

struct boidWall {
  int xpos;
  int ypos;
};
typedef struct boidWall boidWall;

struct wallContainer {
  boidWall * wallArr;
  int size;
  int alloc;
};

struct boid {
	int xpos;
	int ypos;
	directionVector velocity;
	int active;
};

typedef struct boid boid;


struct boidContainer {
	boid * boidArr;
	int size;
	int alloc;
};

//typedef struct boidContainer boidContainer;


//List prototypes for boid acceleration calculation
//All should return directional vectors for acceleration calculation
void moveBoid(const goalContainer * const goals, wallContainer * wallList, boidContainer * boidlist, const boidContainer * const effectingBoids, int index);
directionVector cohesion(const boidContainer * const boidlist,const boidContainer * const effectList, int index);
directionVector aversion(const boidContainer * const boidlist, wallContainer * walls,const boidContainer * const effectList, int index);
directionVector alignment(const boidContainer * const boidlist,const boidContainer * const effectList, int index);
directionVector moveToExit(const goalContainer * const goals, const boidContainer * const boidlist, int index);

directionVector getGoalDistance(int goalx, int goaly, const boid * const b);

int inSlice(boid thisBoid, int x, int y, int X, int Y);

#define CONTAINEREXTEND 10

//boidContainer functions
int boidInsert(boidContainer * container, boid * insert);
int boidRemove(boidContainer * container, int index);

// wallContainer function
int wallInsert(wallContainer * walls, boidWall * insert);

#endif
