#ifndef SETUP_H
#define SETUP_H
struct boidContainer;
typedef struct boidContainer boidContainer;
typedef struct wallContainer wallContainer;

struct goalContainer {
	//pos is an array of positions (integer array with x and y)
	int ** pos;
	int size;
	int alloc;
};

typedef struct goalContainer goalContainer;

void setupSimulation(char * fileName, boidContainer * boids, wallContainer * walls, goalContainer * goals, short *** board, short *** blank, unsigned int * width, unsigned int * height);
void addGoal(goalContainer * goals, int x, int y);

#endif
