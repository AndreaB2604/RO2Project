#include "tsp.h"

void insert(double *values, int *positions, double value, int pos, int size);

int TSP_nn_grasp_heur(instance *inst)
{
	int *tour;
	double curr_heur_sol;

	grasp_heur(inst, 0, 0.5, 4, &tour, &curr_heur_sol);

	printf("\nTour: ");
	for(int i = 0; i < inst->nnodes; i++)
		printf("%d, ", tour[i]+1);
	printf("\n");
	
	printf("current heur sol = %f\n", curr_heur_sol);

	free(tour);

	return 0;
}

// nnc: is the number of possible choices in the GRASP heuristic
void grasp_heur(instance *inst, int init_node, double prob, int nnc, int **tour, double *obj_val)
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

	*tour = (int *) calloc(inst->nnodes, sizeof(int));
	(*tour)[0] = init_node;
	
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
		for(int j = 1; j < inst->nnodes; j++)
		{
			if(!visited[j])
			{
				double curr_dist = dist((*tour)[i-1], j, inst);
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
			(*tour)[i] = neigh_pos[0];
			visited[neigh_pos[0]] = 1;
		}
		else
		{
			int node_chosen = random() % node_left;
			(*tour)[i] = neigh_pos[node_chosen];
			visited[neigh_pos[node_chosen]] = 1;
		}
		for(int j=0; j < node_left; j++)
		{
			printf("%d, ", neigh_pos[j]+1);
		}
		
		printf("\n");
	}

	*obj_val = tour_dist(*tour, inst->nnodes, inst);

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
