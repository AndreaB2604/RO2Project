#include "tsp.h"

int TSP_heur_2opt(instance *inst)
{
	int *tour = (int *) calloc(inst->nnodes, sizeof(int));
	double best_heur_sol = DBL_MAX;

	double tmp_sol = DBL_MAX;
	int *tmp_tour = (int *) calloc(inst->nnodes, sizeof(int));
	for(int i = 0; i < inst->nnodes; i++)
	{
		grasp_heur(inst, i, 0, 4, tmp_tour, &tmp_sol);
		if(tmp_sol < best_heur_sol)
		{
			best_heur_sol = tmp_sol;
			for(int j = 0; j < inst->nnodes; j++)
			{
				tour[j] = tmp_tour[j];
			}
		}
	}

	double nn_dist = tour_dist(inst, tour);
	if(VERBOSE >= 100)
	{
		printf("Obj val after nearest neighbourhood = %f\n", nn_dist);
	}
	
	two_opt(inst, tour, tmp_tour, inst->time_limit);
	
	double two_opt_dist = tour_dist(inst, tmp_tour);
	if(VERBOSE >= 100)
	{
		printf("Time limit of %.3f seconds reached\n", inst->time_limit);
		printf("Obj val after 2-OPT = %f\n", two_opt_dist);
	}

	// saving in the inst->best_sol for the plot
	int cur_numcols = inst->nnodes * (inst->nnodes - 1) / 2;
	inst->best_sol = (double *) calloc(cur_numcols, sizeof(double));
	for(int i = 0; i < inst->nnodes; i++)
	{
		int idx = xpos(tmp_tour[i], tmp_tour[(i+1)%inst->nnodes], inst);
		inst->best_sol[idx] = 1.0;
	}

	free(tmp_tour);
	free(tour);

	return 0;
}

void two_opt(instance *inst, int *init_sol, int *up_sol, double time_limit)
{
	int n = inst->nnodes;
	int *curr_tour = (int *) calloc(inst->nnodes, sizeof(int));
	int *tmp_tour = (int *) calloc(inst->nnodes, sizeof(int));

	for(int i = 0; i < n; i++)
	{
		curr_tour[i] = init_sol[i];
	}
	double curr_tour_dist = tour_dist(inst, curr_tour);

	int done = 0;
	unsigned long start = microseconds();
	while(!done)
	{
		double elasped = (microseconds() - start)/1000000.0;
		if(elasped > time_limit)
		{
			done = 1;
		}
		else
		{
			for(int i = 1; i < n-1; i++) 
			{
				for(int j = i+1; j < n; j++) 
				{
					swap_edges(inst, curr_tour, tmp_tour, i, j);
					double tmp_tour_dist = tour_dist(inst, tmp_tour);

					if(tmp_tour_dist < curr_tour_dist)
					{
						curr_tour_dist = tmp_tour_dist;
						for(int k = 0; k < n; k++)
						{
							curr_tour[k] = tmp_tour[k];
						}
					}
				}
			}
		}
	}
	
	for(int i = 0; i < n; i++)
	{
		up_sol[i] = curr_tour[i];
	}

	free(tmp_tour);
	free(curr_tour);
}

void swap_edges(instance *inst, int *old_tour, int *new_tour, int i, int j)
{
	int n = inst->nnodes;
	for(int k = 0; k < i; k++)
	{
		new_tour[k] = old_tour[k];
	}
	for(int k = i, pos = 0; k <= j; k++, pos++)
	{
		new_tour[k] = old_tour[j-pos];
	}

	for(int k = j+1; k < n; k++)
	{
		new_tour[k] = old_tour[k];
	}	
}