#ifndef TSP_H_
#define TSP_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <cplex.h>

#define VERBOSE 10000 // printing level  (=10 only incumbent, =20 little output, =50-60 good, =70 verbose, >=100 cplex log) 
#define TOLERANCE 0.5

typedef struct
{
	// input data
	int nnodes;
	double *xcoord; 								// they could be integer
	double *ycoord;

	// parameters
	double time_limit; 								// long can be used
	char *input_file;
	double *best_sol;								// best solution available
} instance;


/* PROTOTYPES */

// defined in utilities.c
void free_instance(instance *inst);
void parse_command_line(int argc, char** argv, instance *inst);
void print_error(const char *err);
void print_plot(instance *inst, char *plot_file_name);
void read_input(instance *inst);
void print_plot_compact(instance *inst, char *plot_file_name);

// defined in tsp.c
void build_model(instance *inst, CPXENVptr env, CPXLPptr lp);
double dist(int i, int j, instance *inst);
int TSPopt(instance *inst);
int xpos(int i, int j, instance *inst);

// defined in tsp_compact.c
void build_model_compact(instance *inst, CPXENVptr env, CPXLPptr lp);
int TSPopt_compact(instance *inst);
int xpos_compact(int i, int j, instance *inst);

#endif
