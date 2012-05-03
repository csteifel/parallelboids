//Master include file

#ifndef INC_H
#define INC_H
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <string.h>
#include <mpi.h>
#include "setup.h"
#include "boid.h"

int rank, numranks;
int widthSlice = 0, heightSlice = 0, widthOffset = 0, heightOffset = 0;


#define MAX(a,b) (((a)>(b))?(a):(b))

#endif
