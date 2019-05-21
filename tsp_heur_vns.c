#include "tsp.h"

int insert_vertex(int *v, int count, int value);
void random_vertexes(instance *inst, int *v, int vsize);

int TSP_heur_vns(instance *inst)
{
	int *x;

	two_approx_algorithm_TSP(inst, &x);
	double best_curr_val = tour_dist(inst, x);

	int *x_first = (int *) calloc(inst->nnodes, sizeof(int));

	int done = 0;
	int k = 2; // k = n means swap random of n edges 
	unsigned long start = microseconds();
	
	while(!done)
	{
		double elasped = (microseconds() - start)/1000000.0;
		if(elasped > inst->time_limit)
		{
			printf("VNS: Time limit of %.3f seconds reached\n", inst->time_limit);
			done = 1;
		}
		else
		{
			if(k == 2)
			{
				int v[2] = {INT_MAX, INT_MAX};
				random_vertexes(inst, v, k);
				swap_two_edges(inst, x, x_first, v[0], v[1]);
			}
			else if(k == 3)
			{
				int v[3] = {INT_MAX, INT_MAX, INT_MAX};
				random_vertexes(inst, v, k);
				swap_three_edges(inst, x, x_first, v[0], v[1], v[2], random() % 4);
			}
			else if(k == 4)
			{
				int v[2] = {INT_MAX, INT_MAX};
				random_vertexes(inst, v, k);
				swap_two_edges(inst, x, x_first, v[0], v[1]);
				v[0] = INT_MAX; v[1] = INT_MAX;
				random_vertexes(inst, v, k);
				swap_two_edges(inst, x, x_first, v[0], v[1]);
			}

			printf("Obj-val before 2-OPT of x = %f\n", tour_dist(inst, x));
			printf("Obj-val before 2-OPT of x_first = %f\n", tour_dist(inst, x_first));

			two_opt(inst, x_first, x_first, DBL_MAX);

			printf("Obj-val after 2-OPT of x_first = %f\n", tour_dist(inst, x_first));

			double x_first_val = tour_dist(inst, x_first);
			if(x_first_val < best_curr_val)
			{
				k = 2;
				best_curr_val = x_first_val;
				for(int i = 0; i < inst->nnodes; i++)
				{
					x[i] = x_first[i];
				}
			}
			else
			{
				k = (k == 4)? k : k+1; 
			}
		}
	}

	for(int i = 0; i < inst->nnodes; i++)
	{
		printf("%d, ", x[i]);
	}
	printf("\n");
	
	printf("Ci passo 1\n");
	// saving in the inst->best_sol for the plot
	int cur_numcols = inst->nnodes * (inst->nnodes - 1) / 2;
	inst->best_sol = (double *) calloc(cur_numcols, sizeof(double));
	for(int i = 0; i < inst->nnodes; i++)
	{
		int idx = xpos(x[i], x[(i+1)%inst->nnodes], inst);
		inst->best_sol[idx] = 1.0;
	}

	printf("Ci passo 2\n");

	free(x_first);
	free(x);
	
	printf("Ci passo 3\n");

	return 0;
}

void random_vertexes(instance *inst, int *v, int vsize)
{
	struct timeval time;
	gettimeofday(&time, NULL);
	srand(time.tv_usec);
	int rand = random() % inst->nnodes;

	v[0] = rand;
	int count = 1;

	while(count < vsize)
	{
		gettimeofday(&time, NULL);
		srand(time.tv_usec);
		rand = random() % inst->nnodes;
		if(!insert_vertex(v, count, rand))
		{
			count++;
		}
	}
}

int insert_vertex(int *v, int count, int value)
{
	int i;
	for(i = 0; i < count && v[i] <= value; i++);
	
	if(v[i] == value) return 1;
	
	for(int j = count; j > i; j--)
	{
		v[j] = v[j-1];
	}
	v[i] = value;

	return 0;
}