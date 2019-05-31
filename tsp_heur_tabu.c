#include "tsp.h"

#define MIN(a, b) ((a<b)? a : b) 
#define MAX(a, b) ((a>b)? a : b) 

void two_opt_tabu(instance *inst, int *init_sol, double best_sol, double *incumbent_value, move *tabu_list, int *tabu_size, const int TABU_DURATION);

int TSP_heur_tabu(instance *inst)
{
	int *x;

	const int TLL = inst->nnodes; // Tabu List Length
	const int TABU_DURATION = ((inst->nnodes / 3) > 100)? 100 : (inst->nnodes/3 - 1);

	FILE *file;
	if(!BLADE)
	{
		file = fopen("plot_heur/tabu_search.txt", "w");
		fprintf(file, "TABU SEARCH\n");
	}

	two_approx_algorithm_TSP(inst, &x);
	double best_curr_val = tour_dist(inst, x);
	
	if(VERBOSE >= 100)
	{
		printf("Obj val after the 2-Approximation algorithm = %f\n", best_curr_val);
	}

	int *x_first = (int *) calloc(inst->nnodes, sizeof(int));

	//int k = 2; // k = n means swap random of n edges 
		
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

	if(VERBOSE >= 100)
	{
		printf("Obj val after the 2-OPT = %f\n", best_curr_val);
	}

	// x_first is the current solution
	for(int i = 0; i < inst->nnodes; i++)
	{
		x_first[i] = x[i];
	}
	move tabu_list[TLL];
	for(int i = 0; i < TLL; i++)
	{
		tabu_list[i].node = -1;
		tabu_list[i].delta = DBL_MAX;
		tabu_list[i].duration = -1;
	}
	int tabu_size = 0;
	int done = 0;
	double incumbent_dist = best_curr_val;
	int iter = 0;
	unsigned long start = microseconds();
	while(!done)
	{
		double elasped = (microseconds() - start)/1000000.0;
		if(BLADE)
		{
			for(int a=0; a<3; ++a)
			{
				double ub = (double) ((a+1)*limit/3);
				if((!flag[a]) && (elasped >= ub))
				{
					printf("solution value at %d is: %f\n", (a+1), best_curr_val);
					flag[a] = 1;
					break;
				}
			}
		}
		if(elasped > inst->time_limit)
		{
			if(VERBOSE >= 100)
			{
				printf("TABU SEARCH: Time limit of %.3f seconds reached\n", inst->time_limit);
			}
			done = 1;
		}
		else
		{
			two_opt_tabu(inst, x_first, best_curr_val, &incumbent_dist, tabu_list, &tabu_size, TABU_DURATION);
			if(iter > 33 && incumbent_dist > best_curr_val)
			{
				srandom((unsigned int)microseconds());
				int n1 = random()%inst->nnodes;
				int n2 = random()%inst->nnodes;
				int tmp = MIN(n1, n2);
				n2 = MAX(n1, n2);
				n1 = tmp;
				swap_two_edges_in_place(x_first, n1, n2);
				incumbent_dist = tour_dist(inst, x_first);
				iter = 0;
			}
			if(incumbent_dist < best_curr_val)
			{
				iter = 0;
				best_curr_val = incumbent_dist;
				for(int i = 0; i < inst->nnodes; i++)
				{
					x[i] = x_first[i];
				}
				if(!BLADE)
				{
					fprintf(file, "%f %f\n", best_curr_val, ((microseconds()-start)/1000000.0));
				}
			}
			else
			{
				iter++;
			}
		}
		if(VERBOSE >= 1000)
		{
			printf("TABU: Obj-val after the move = %f\n", tour_dist(inst, x_first));
			printf("TABU: best obj-val after the move with tour_dist = %f\n", tour_dist(inst, x));
		}
	}

	if(VERBOSE > 1000)
	{
		for(int i = 0; i < inst->nnodes; i++)
		{
			printf("%d, ", x[i]);
		}
		printf("\n");	
	}
	if(!BLADE)
	{
		fprintf(file, "%f %f\n", tour_dist(inst, x), ((microseconds()-start)/1000000.0));
	}
	if(VERBOSE >= 100)
	{
		printf("Obj val after the Tabu Search = %f\n", tour_dist(inst, x));
	}

	// saving in the inst->best_sol for the plot
	int cur_numcols = inst->nnodes * (inst->nnodes - 1) / 2;
	inst->best_sol = (double *) calloc(cur_numcols, sizeof(double));
	for(int i = 0; i < inst->nnodes; i++)
	{
		int idx = xpos(x[i], x[(i+1)%inst->nnodes], inst);
		inst->best_sol[idx] = 1.0;
	}

	fclose(file);
	free(x_first);
	free(x);
	
	return 0;
}

void two_opt_tabu(instance *inst, int *init_sol, double best_sol, double *incumbent_value, move *tabu_list, int *tabu_size, const int TABU_DURATION)
{
	move best_move = {-1, DBL_MAX, 0};
	
	int n = inst->nnodes;
	
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
			//printf("Delta = %f\n", delta);best_curr_val
			if(delta < best_delta)
			{
				int is_tabu = 0;
				for(int k = 0; (k < *tabu_size) && (!is_tabu); k++)
				{
					if(tabu_list[k].node == j)
					{
						//printf("tabu_list[k].node = %d, init_sol[j] = %d\n", tabu_list[k].node, init_sol[j]);
						if(((*incumbent_value) + delta) < best_sol)
						{
							best_delta = delta;
							best_move.node = j;
							best_move.delta = delta;
							best_move.duration = TABU_DURATION;
							tabu_pos = k;
							to_swap = i;
						}
						is_tabu = 1;
					}
				}
				if(!is_tabu)
				{
					best_delta = delta;
					best_move.node = j;
					best_move.delta = delta;
					best_move.duration = TABU_DURATION;
					tabu_pos = -1;
					to_swap = i;
				}
			}
		}
	}
	/*printf("best_delta = %f\n", best_delta);
	printf("tabu_pos = %d\n", tabu_pos);
	printf("to_swap = %d\n", to_swap);
	*/

	int move_to_remove = -1;
	for(int i = 0; i < *tabu_size; i++)
	{
		if(i == tabu_pos)
		{
			tabu_list[i].duration = TABU_DURATION;
		}
		else
		{
			tabu_list[i].duration--;
		}
		
		if(tabu_list[i].duration == 0)
		{
			move_to_remove = i;
		}		
	}

	if(tabu_pos == -1 && move_to_remove == -1) 		// no zero element and best move is not in the tabu list
	{
		tabu_list[(*tabu_size)].node = best_move.node;
		tabu_list[(*tabu_size)].duration = best_move.duration;
		tabu_list[(*tabu_size)++].delta = best_move.delta;
	}
	else if(tabu_pos == -1 && move_to_remove >= 0)	// a zero element and best move is not in the tabu list	
	{
		tabu_list[move_to_remove].node = best_move.node;
		tabu_list[move_to_remove].duration = best_move.duration;
		tabu_list[move_to_remove].delta = best_move.delta;
	}
	else if (tabu_pos >= 0 && move_to_remove >= 0)	// a zero element and the best move is in the tabu list
	{
		tabu_list[move_to_remove].node = tabu_list[--(*tabu_size)].node;
		tabu_list[move_to_remove].duration = tabu_list[(*tabu_size)].duration;
		tabu_list[move_to_remove].delta = tabu_list[(*tabu_size)].delta;
	}

	//printf("best_move.delta = %f\n", best_move.delta);
	//printf("tabu size = %d\n", *tabu_size);
	/*if((*tabu_size) == 100)
	{
		for(int i = 0; i < *tabu_size; i++)
			printf("move %d: %d, %f, %d\n", i, tabu_list[i].node, tabu_list[i].delta, tabu_list[i].duration);
		exit(0);
	}*/

	/*printf("\nTHE TABU LIST of size = %d\n", *tabu_size);
	for(int i = 0; i < *tabu_size; i++)
	{
		printf("tabu_list[i].node = %d\n", tabu_list[i].node); 
		printf("tabu_list[i].delta = %f\n", tabu_list[i].delta); 
		printf("tabu_list[i].duration = %d\n", tabu_list[i].duration);
	}*/

	swap_two_edges_in_place(init_sol, to_swap, best_move.node);
	(*incumbent_value) += best_move.delta;
}
