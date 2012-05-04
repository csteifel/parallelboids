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


#if defined(__i386__)

static __inline__ unsigned long long rdtsc(void)
{
	unsigned long long int x;
	__asm__ volatile (".byte 0x0f, 0x31" : "=A" (x));
	return x;
}
#elif defined(__x86_64__)


static __inline__ unsigned long long rdtsc(void)
{
	unsigned hi, lo;
	__asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
	return ( (unsigned long long)lo)|( ((unsigned long long)hi)<<32 );
}

#elif defined(__powerpc__)
static __inline__ unsigned long long rdtsc(void)
{
	unsigned long long int result=0;
	unsigned long int upper, lower,tmp;
	      __asm__ volatile(
		"0:                  \n"
		"\tmftbu   %0           \n"
		"\tmftb    %1           \n"
		"\tmftbu   %2           \n"
		"\tcmpw    %2,%0        \n"
		"\tbne     0b         \n"
		: "=r"(upper),"=r"(lower),"=r"(tmp)
		);
	result = upper;
	result = result<<32;
	result = result|lower;

	return(result);
}
#endif

unsigned long long execTime, setupTime, sendTimeTotal, sendTime, recvTimeTotal, recvTime, stepTime;
unsigned long long sendTotal, recvTotal, stepTotal;
unsigned long long sendAmount, sendAmountTotal, recvAmount, recvAmountTotal;


#endif
