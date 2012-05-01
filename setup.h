struct boidContainer;
typedef struct boidContainer boidContainer;

struct goalContainer {
	//pos is an array of positions (integer array with x and y)
	int ** pos;
	int size;
	int alloc;
};

typedef struct goalContainer goalContainer;

void setupSimulation(char * fileName, boidContainer * boids, goalContainer * goals, short *** board, short *** blank, unsigned int * width, unsigned int * height);
void addGoal(goalContainer * goals, int x, int y);
