#include "tsp.h"

const int TABU_DURATION = 100;

void two_opt_tabu(instance *inst, int *init_sol, double best_sol, move best_move, move *tabu_list, int *tabu_size);

int TSP_heur_tabu(instance *inst)
{
	int *x;

	const int TLL = 100; // Tabu List Length
	two_approx_algorithm_TSP(inst, &x);
	double best_curr_val;
	
	if(VERBOSE >= 100)
	{
		printf("Obj val after the 2-Approximation algorithm = %f\n", best_curr_val);
	}

	int *x_first = (int *) calloc(inst->nnodes, sizeof(int));

	int done = 0;
	int k = 2; // k = n means swap random of n edges 
	unsigned long start = microseconds();
	
	//----------- for BLADE purposes-------------//
	int flag[3] = {0};
	int limit = inst->time_limit;
	if(BLADE)
	{
		printf("solution value at %d is: %f\n", flag[0], best_curr_val);
	}
	//-------------------------------------------//

	//Local minima in the neighbourhood
	two_opt(inst, x, DBL_MAX);
	best_curr_val = tour_dist(inst, x);

	// x_first is the current solution
	for(int i = 0; i < inst->nnodes; i++)
	{
		x_first[i] = x[i];
	}
	move tabu_list[TLL];  
	move candidates[TLL+1];  // we chose TLL+1 based on empirical facts..
	done = 0;
	while(!done)
	{
		//two_opt_tabu(inst, x_first, candidates, TLL+1);
	}
	

	free(x_first);
	free(x);
	
	return 0;
}

void two_opt_tabu(instance *inst, int *init_sol, double best_sol, move best_move, move *tabu_list, int *tabu_size)
{
	int n = inst->nnodes;
	
	double init_sol_dist = tour_dist(inst, init_sol);
	double best_delta = DBL_MAX;
	int tabu_pos = -1;
	int to_swap = -1;
	for(int i = 1; (i < n-1); i++) 
	{
		for(int j = i+1; j < n; j++) 
		{
			double pos_term = dist(init_sol[i-1], init_sol[j], inst) + dist(init_sol[i], init_sol[(j+1)%n], inst);
			double neg_term = dist(init_sol[i-1], init_sol[i], inst) + dist(init_sol[j], init_sol[(j+1)%n], inst);
			double delta = pos_term - neg_term;
			if(delta < best_delta)
			{
				int is_tabu = 0;
				for(int k = 0; k < *tabu_size && !is_tabu; k++)
				{
					if(tabu_list[k].node == j)
					{
						if(init_sol_dist + delta < best_sol)
						{
							best_delta = delta;
							best_move.node = i;
							best_move.delta = delta;
							best_move.duration = TABU_DURATION;
							tabu_pos = k;
							to_swap = j;
						}
						is_tabu = 1;
					}
				}
				if(!is_tabu)
				{
					best_delta = delta;
					best_move.node = i;
					best_move.delta = delta;
					best_move.duration = TABU_DURATION;
					tabu_pos = -1;
					to_swap = j;
				}
			}
		}
	}

	for(int i = 0; i < *tabu_size; i++)
	{
		if(i == tabu_pos)
			tabu_list[i].duration = TABU_DURATION;
		else
			tabu_list[i].duration--;
	}
	if(tabu_pos == -1)
	{
		tabu_list[(*tabu_size)++] = best_move;
	}
	swap_two_edges_in_place(init_sol, best_move.node, to_swap);
}
