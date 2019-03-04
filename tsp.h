#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FILE_SIZE 1000 // maximum file size we accept

typedef struct
{
	// input data
	int nnodes;
	double *xcoord; 								// they could be integer
	double *ycoord;

	// parameters
	double time_limit; 								// long can be used
	char input_file[FILE_SIZE];
	double *best_sol; 								// best solution available
} instance;

