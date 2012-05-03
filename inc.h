#ifndef INC_H
#define INC_H


//Master include file


#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <string.h>
#include <mpi.h>
#include "setup.h"
#include "boid.h"

int rank, numranks;
int widthSlice, heightSlice, widthOffset, heightOffset;

#define MAX(a,b) (((a)>(b))?(a):(b))

#endif
