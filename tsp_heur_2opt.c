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
	if(VERBOSE >= 100 || BLADE)
	{
		printf("Obj val before 2-OPT = %f\n", nn_dist);
	}
	
	two_opt(inst, tour, inst->time_limit);
	
	double two_opt_dist = tour_dist(inst, tour);
	if(VERBOSE >= 100 || BLADE)
	{
		printf("Obj val after 2-OPT = %f\n", two_opt_dist);
	}
	
	// saving in the inst->best_sol for the plot
	int cur_numcols = inst->nnodes * (inst->nnodes - 1) / 2;
	inst->best_sol = (double *) calloc(cur_numcols, sizeof(double));
	for(int i = 0; i < inst->nnodes; i++)
	{
		int idx = xpos(tour[i], tour[(i+1)%inst->nnodes], inst);
		inst->best_sol[idx] = 1.0;
	}

	free(tmp_tour);
	free(tour);

	return 0;
}

void two_opt(instance *inst, int *init_sol, double time_limit)
{
	int n = inst->nnodes;
	int *tmp_tour = (int *) calloc(inst->nnodes, sizeof(int));

	double init_sol_dist = tour_dist(inst, init_sol);
	double prev_sol = init_sol_dist;

	//----------- for BLADE purposes-------------//
	int flag[3] = {0};
	int limit = inst->time_limit;
	
	int done = 0;
	unsigned long start = microseconds();
	do
	{
		double elasped = (microseconds() - start)/1000000.0;
		if(BLADE)
		{
			for(int a=0; a<3; ++a)
			{
				double ub = (double) ((a+1)*limit/3);
				if((!flag[a]) && (elasped >= ub))
				{
					printf("solution value at %d is: %f\n", (a+1), init_sol_dist);
					flag[a] = 1;
					break;
				}
			}
		}
		if(elasped > time_limit)
		{
			if(VERBOSE >= 100)
			{
				printf("2-OPT: Time limit of %.3f seconds reached\n", time_limit);
			}
			done = 1;
		}
		else
		{
			for(int i = 1; i < n-1; i++) 
			{
				for(int j = i+1; j < n; j++) 
				{
					double pos_term = dist(init_sol[i-1], init_sol[j], inst) + dist(init_sol[i], init_sol[(j+1)%n], inst);
					double neg_term = dist(init_sol[i-1], init_sol[i], inst) + dist(init_sol[j], init_sol[(j+1)%n], inst);
					double delta = pos_term - neg_term;
					if(delta < 0)
					{						
						init_sol_dist += delta;
						swap_two_edges(inst, init_sol, tmp_tour, i, j);
						for(int k = 0; k < n; k++)
						{
							init_sol[k] = tmp_tour[k];
						}
					}
				}
			}
		}
		if(((int)init_sol_dist) == ((int)prev_sol))
		{
			done = 1;
		}
		else
		{
			prev_sol = init_sol_dist;
		}
	}
	while(!done);

	free(tmp_tour);
}

void swap_two_edges(instance *inst, int *old_tour, int *new_tour, int i, int j)
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

void swap_three_edges(instance *inst, int *old_tour, int *new_tour, int i, int j, int h, int type)
{
	int n = inst->nnodes;
	for(int k = 0; k < i; k++)
	{
		new_tour[k] = old_tour[k];
	}
	// reference to paper "combining 2opt 3 opt...": figure (1.e) 
	if(type == 0)
	{
		for(int k = i, count = 0; k <= h-j+i-1; k++, count++)
		{
			new_tour[k] = old_tour[j+1+count];
		}
		for(int k = h-j+i, count = 0; k <= h; k++, count++)
		{
			new_tour[k] = old_tour[j-count];
		}
	}
	// reference to paper: figure (1.f)
	else if(type == 1)
	{
		for(int k = i, count = 0; k <= h-j+i-1; k++, count++)
		{
			new_tour[k] = old_tour[h-count];
		}
		for(int k = h-j+i, count = 0; k <= h; k++, count++)
		{
			new_tour[k] = old_tour[i+count];
		}
	}
	// reference to paper: figure (1.g)
	else if(type == 2)
	{
		for(int k = i, count = 0; k <= j; k++, count++)
		{
			new_tour[k] = old_tour[j-count];
		}
		for(int k = j+1, count = 0; k <= h; k++, count++)
		{
			new_tour[k] = old_tour[h-count];
		}
	}
	// reference to paper: figure (1.h)
	else if(type == 3)
	{
		for(int k = i, count = 0; k <= h-j+i-1; k++, count++)
		{
			new_tour[k] = old_tour[j+1+count];
		}
		for(int k = h-j+i, count = 0; k <= h; k++, count++)
		{
			new_tour[k] = old_tour[i+count];
		}
	}

	for(int k = h+1; k < n; k++)
	{
		new_tour[k] = old_tour[k];
	}
}
