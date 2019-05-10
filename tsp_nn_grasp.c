#include "tsp.h"

void insert(double *values, int *positions, double value, int pos, int size);

int TSP_nn_grasp_heur(instance *inst)
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

	if(VERBOSE >= 0)
	{
		printf("\nBest tour found without GRASP:\n");
		for(int i = 0; i < inst->nnodes; i++)
			printf("%d, ", tour[i]+1);
		printf("\n");
		
		printf("Length of the tour = %f\n", best_heur_sol);
	}

	// using the GRASP
	double prob = 0.5;
	for(int i = 0; i < inst->nnodes; i++)
	{
		for(int j = 0; j < 10; j++)
		{
			grasp_heur(inst, i, prob, 4, tmp_tour, &tmp_sol);
			if(tmp_sol < best_heur_sol)
			{
				best_heur_sol = tmp_sol;
				for(int j = 0; j < inst->nnodes; j++)
				{
					tour[j] = tmp_tour[j];
				}
			}	
		}
	}

	printf("\nBest tour found with GRASP:\n");
	for(int i = 0; i < inst->nnodes; i++)
		printf("%d, ", tour[i]+1);
	printf("\n");
	
	printf("Length of the tour = %f\n", best_heur_sol);

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

// nnc: is the number of possible choices in the GRASP heuristic
void grasp_heur(instance *inst, int init_node, double prob, int nnc, int *tour, double *obj_val)
{
	if(prob < 0 || prob > 1)
	{
		print_error(" in grasp_heur: probability must be in [0,1]");
	}
	if(nnc <= 0)
	{
		print_error(" in grasp_heur: the number of possible choices (nnc) is smaller than or equal to zero");
	}
	if(nnc > inst->nnodes)
	{
		print_error(" in grasp_heur: the number of possible choices (nnc) is greater than the number of nodes");
	}

	*tour = init_node;
	
	double rand;
	struct timeval time;

	double neigh_dist[nnc];
	int neigh_pos[nnc];

	int *visited = (int *) calloc(inst->nnodes, sizeof(int));
	visited[init_node] = 1;

	for(int i = 1; i < inst->nnodes; i++)
	{
		// initialisation to default
		int node_left = 0; // this will be the number of not yet entered nodes
		for(int i = 0; i < nnc; i++)
		{
			neigh_dist[i] = DBL_MAX;
			neigh_pos[i] = -1;
		}

		// compute the nearest neighbourhood 
		for(int j = 0; j < inst->nnodes; j++)
		{
			if(!visited[j])
			{
				double curr_dist = dist(tour[i-1], j, inst);
				insert(neigh_dist, neigh_pos, curr_dist, j, nnc);
				node_left++;
			}
		}

		node_left = (node_left < nnc)? node_left : nnc;

		gettimeofday(&time, NULL);
		srand(time.tv_usec);
		rand = (double) random() / RAND_MAX;
		if(rand >= prob) // don't use GRASP
		{
			tour[i] = neigh_pos[0];
			visited[neigh_pos[0]] = 1;
		}
		else
		{
			gettimeofday(&time, NULL);
			srand(time.tv_usec);
			int node_chosen = random() % node_left;
			tour[i] = neigh_pos[node_chosen];
			visited[neigh_pos[node_chosen]] = 1;
		}
	}

	*obj_val = tour_dist(tour, inst->nnodes, inst);

	free(visited);
}

void insert(double *values, int *positions, double value, int pos, int size)
{
	int i;
	for(i = 0; (i < size) && values[i] <= value; i++);
	
	if(i == size) // cannot insert the node anywhere
		return;

	for(int j = size-1; j > i; j--)
	{
		values[j] = values[j-1];
		positions[j] = positions[j-1];
	}
	values[i] = value;
	positions[i] = pos;
}
