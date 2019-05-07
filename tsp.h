#ifndef TSP_H_
#define TSP_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <cplex.h>
#include <cplexx.h>
#include <sys/time.h>
#include <float.h>

#define VERBOSE 100 // printing level  (=10 only incumbent, =20 little output, =50-60 good, =70 verbose, >=100 cplex log) 
#define TOLERANCE 0.5

typedef struct
{
	// input data
	int nnodes;
	double *xcoord; 						// they could be integer
	double *ycoord;

	// parameters
	double time_limit; 						// long can be used
	char *dist_type;
	char *input_file;
	char *model_type;
	double *best_sol;						// best solution available
	long random_seed;						// random seed
} instance;


/* PROTOTYPES */

// defined in utilities.c
void connected_components(instance *inst, double *best_sol, int columns_number, int *nodes_components, int *components_values, int *number_cc);
double dist(int i, int j, instance *inst);
void free_instance(instance *inst);
unsigned long microseconds();
void parse_command_line(int argc, char** argv, instance *inst);
void print_error(const char *err);
void print_plot(instance *inst, char *plot_file_name);
void read_input(instance *inst);

// defined in tsp.c
void build_model(instance *inst, CPXENVptr env, CPXLPptr lp);
int TSPopt(instance *inst);
int xpos(int i, int j, instance *inst);

// defined in tsp_sec_callback.c
int myseparation(instance *inst, double *xtar, CPXCENVptr env, void *cbdata, int wherefrom);
int TSPopt_sec_callback(instance *inst);

// defined in tsp_sec_loop.c
int sec_loop(CPXENVptr env, CPXLPptr lp, instance *inst);
int TSPopt_sec_loop(instance *inst);

// defined in tsp_mtz.c
void build_model_mtz(instance *inst, CPXENVptr env, CPXLPptr lp);
int TSPopt_mtz(instance *inst);
int xpos_mtz(int i, int j, instance *inst);

// defined in tsp_compact_custom
void build_model_compact_custom(instance *inst, CPXENVptr env, CPXLPptr lp);
int TSPopt_compact_custom(instance *inst);
int zpos_compact_custom(int i, int j, instance *inst);

// defined in tsp_hard_fixing
int TSP_heur_hf(instance *inst);

#endif
