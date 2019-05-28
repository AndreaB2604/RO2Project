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
#include <limits.h>
#include <concorde.h>

#define VERBOSE 100 // printing level  (=10 only incumbent, =20 little output, =50-60 good, =70 verbose, >=100 cplex log) 
#define TOLERANCE 0.5
#define BLADE 0 // 1 to require the prints on blade 0 otherwise 
#define dettime 0 // 0 = use non deterministic time (seconds), 1 = use ticks

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

typedef struct
{
	CPXCENVptr env;
	void *cbdata;
	int wherefrom;
	int *useraction_p;
	instance *inst;

} cc_instance;

typedef struct 
{
	int node;
	double delta; // improvement of the move to the obj function
	int duration; // duration of the move in the tabu list
} move;

struct node
{
	int number;
	struct node *firstchild;
	struct node *nextsibling;
};

/* PROTOTYPES */

// defined in utilities.c
void connected_components(instance *inst, double *best_sol, int columns_number, int *nodes_components, int *components_values, int *number_cc);
double dist(int i, int j, instance *inst);
void free_instance(instance *inst);
void init_instance(instance *inst);
unsigned long microseconds();
void parse_command_line(int argc, char** argv, instance *inst);
void print_error(const char *err);
void print_plot(instance *inst, char *plot_file_name);
void read_input(instance *inst);
double tour_dist(instance *inst, int *v);

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
void add_children(struct node *node, int *prev, int *visited, int size);
void free_tree(struct node **node);
void pre_order_visit(struct node *node, int *tour, int *tour_idx);
void prim_dijkstra_MST(instance *inst, int **pred);
void random_fixing(CPXENVptr env, CPXLPptr lp, double *sol_heur, int size, double prob);
void set_default_lb(CPXENVptr env, CPXLPptr lp, int size);
void two_approx_algorithm_TSP(instance *inst, int **approx_tour_ptr);
int TSP_heur_hf(instance *inst);

// defined in tsp_local_branching.c
int TSP_heur_lb(instance *inst);

// defined in tsp_usr_callback.c
int TSPopt_usr_callback(instance *inst);

// defined in tsp_nn_grasp.c
void grasp_heur(instance *inst, int init_node, double prob, int nnc, int *tour, double *obj_val);
void insert(double *values, int *positions, double value, int pos, int size);
int TSP_nn_grasp_heur(instance *inst);

// defined in tsp_heur_2opt.c
void swap_two_edges(instance *inst, int *old_tour, int *new_tour, int i, int j);
void swap_two_edges_in_place(int *v, int i, int j);
void swap_three_edges(instance *inst, int *old_tour, int *new_tour, int i, int j, int h, int type);
int TSP_heur_2opt(instance *inst);
void two_opt(instance *inst, int *sol, double time_limit);

// defined in tsp_heur_vns.c
int TSP_heur_vns(instance *inst);

#endif
