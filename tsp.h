#ifndef TSP_H_
#define TSP_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <cplex.h>

#define VERBOSE 10000 // printing level  (=10 only incumbent, =20 little output, =50-60 good, =70 verbose, >=100 cplex log) 
#define FILE_SIZE 1000 // maximum file size we accept
typedef struct
{
	// input data
	int nnodes;
	double *xcoord; 								// they could be integer
	double *ycoord;

	// parameters
	double time_limit; 								// long can be used
	char *input_file;
	double *best_sol; 								// best solution available
} instance;

#endif
