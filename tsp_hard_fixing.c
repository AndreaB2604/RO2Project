#include "tsp.h"

struct node
{
	int number;
	struct node *firstchild;
	struct node *nextsibling;
};


void add_children(struct node *node, int *prev, int *visited, int size);
void free_tree(struct node *node);
static int CPXPUBLIC mylazycallback(CPXCENVptr env, void *cbdata, int wherefrom, void *cbhandle, int *useraction_p);
void pre_order_visit(struct node *node, int *tour, int *tour_idx);
void prim_dijkstra_MST(instance *inst, int **pred);
void random_fixing(CPXENVptr env, CPXLPptr lp, double *sol_heur, int size, double prob);
void set_default_lb(CPXENVptr env, CPXLPptr lp, int size);
void two_approx_algorithm_TSP(instance *inst, int **approx_tour_ptr);

int TSP_heur_hf(instance *inst)
{
	const int MAX_COUNT = 10;

	double prob_rate[] = {0.9, 0.5, 0.2};
	int prob_rate_max_idx = 2;
	// open cplex model
	int error, status;
	int done = 0;
	//int cur_numrows, cur_numcols;
	//double obj_val;

	CPXENVptr env = CPXopenCPLEX(&error);
	CPXsetintparam(env, CPXPARAM_Read_DataCheck, 1);			// used to check if there are errors while reading data

	CPXLPptr lp = CPXcreateprob(env, &error, "TSP"); 
	if(VERBOSE > 50)
	{
		CPXsetlogfilename(env, "exec_log.txt", "w");			// it saves the log of the computation in exec_compact_log.txt
	}

	// build model
	build_model(inst, env, lp);

	CPXsetintparam(env, CPX_PARAM_MIPCBREDLP, CPX_OFF); // let MIP callbacks work on the original model
	//CPXsetintparam(env, CPX_PARAM_PRELINEAR, 0);
	//CPXsetintparam(env, CPX_PARAM_REDUCE, CPX_PREREDUCE_PRIMALONLY);
	CPXsetlazyconstraintcallbackfunc(env, mylazycallback, inst);
	int ncores = 1;
	CPXgetnumcores(env, &ncores);
	CPXsetintparam(env, CPX_PARAM_THREADS, ncores); // reset after callback

	int *approx_tour = NULL;

	two_approx_algorithm_TSP(inst, &approx_tour);

	double *values = (double *) calloc(inst->nnodes, sizeof(double)); // values of the variables (1.0)
	int *varindices = (int *) calloc(inst->nnodes, sizeof(double)); // indexes of the variables to change
	
	for(int i = 0; i < inst->nnodes; i++)
	{
		values[i] = 1.0;
	}

	double best_obj_val = 0;
	for(int i = 0; i < inst->nnodes; i++)
	{
		int node1 = approx_tour[i];
		int node2 = approx_tour[(i+1)%inst->nnodes];
		varindices[i] = xpos(node1, node2, inst);
		best_obj_val += dist(node1, node2, inst);
		if(VERBOSE >= 1000)
		{
			printf("inserted edge: %d %d\n", node1, node2);
		}
	}
	if(VERBOSE >= 100)
	{
		printf("first solution value: %f\n", best_obj_val);
	}

	int beg = 0;
	int effortlevel = 2;
	if(CPXaddmipstarts(env, lp, 1, inst->nnodes, &beg, varindices, values, &effortlevel, NULL))
	{
		print_error("Failure to add the first solution in CPXaddmipstarts()");
	}

	int cur_numcols = inst->nnodes * (inst->nnodes - 1) / 2;
	inst->best_sol = (double *) calloc(cur_numcols, sizeof(double));
	for(int i = 0; i < inst->nnodes; i++)
	{
		inst->best_sol[varindices[i]] = 1.0;
	}

	int counter = 0;
	int idx_prob_rate = 0;
	double callback_time_limit = 10;	// time-limit of the callback
	unsigned long start = microseconds();
	while(!done)
	{
		// save model
		unsigned long current_time = microseconds();
		double elapsed = (double) (current_time - start) / 1000000;
		if(elapsed >= inst->time_limit)
		{
			if(VERBOSE >= 50)
			{
				printf("Time Limit reached.\n");
			}
			done = 1;
		}
		else
		{
			if(counter >= 10)
			{
				callback_time_limit *= 1.5;
				idx_prob_rate = (idx_prob_rate == prob_rate_max_idx)? idx_prob_rate : idx_prob_rate+1;
			}

			callback_time_limit = (elapsed+callback_time_limit <= inst->time_limit)? callback_time_limit : inst->time_limit-elapsed;
			CPXsetdblparam(env, CPXPARAM_TimeLimit, callback_time_limit);
			random_fixing(env, lp, inst->best_sol, cur_numcols, prob_rate[idx_prob_rate]);
			
			if(status = CPXmipopt(env, lp))
			{
				printf("Status: %d\n", status);
				print_error("Optimisation failed in TSP_heur_hf()");
			}

			// get the best solution and print it
			double current_obj_val = DBL_MAX;
			if(CPXgetobjval(env, lp, &current_obj_val))
			{
				print_error("Failure to read the value of the optimal solution in CPXgetobjval()");
			}
			if(current_obj_val < best_obj_val)
			{
				best_obj_val = current_obj_val;
				counter = 0;
				idx_prob_rate = 0;

				if(CPXgetx(env, lp, inst->best_sol, 0, cur_numcols-1))
				{
					print_error("Failure to read the optimal solution in CPXgetx()");
				}
			}
			else
			{
				counter++;
			}

			set_default_lb(env, lp, cur_numcols);

			// save model
			if(VERBOSE >= 100)
			{
				CPXwriteprob(env, lp, "tsp_hard_fixing.lp", NULL); 
			}

			if(VERBOSE >= 100)
			{
				printf("Incumbent solution: %f\nBest x_heu: %f\n", current_obj_val, best_obj_val);
			}
			// settare tempo della callback < time_limit
			// fissare alcuni lati con la funzione quella la
			// risolvi 
			// prendi l'incumbent e il suo valore
			// confronti con la soluzione corrente ed eventualmente aggiorni
			// ripristinare a zero tutti i bound
			// aumentare counter se la soluzione è la stessa

		}
	}

	free(approx_tour);
	free(values);
	free(varindices);
	return 0;

}

void set_default_lb(CPXENVptr env, CPXLPptr lp, int size)
{
	double zero = 0.0;
	char lb = 'L';

	for(int i = 0; i < size; i++)
	{
		CPXchgbds(env, lp, 1, &i, &lb, &zero);
	}
}

void random_fixing(CPXENVptr env, CPXLPptr lp, double *sol_heur, int size, double prob)
{
	double rand;
	double one = 1.0;
	char lb = 'L';
	struct timeval start;

	if(prob < 0 || prob > 1)
	{
		print_error(" in random_fixing: probability must be in [0,1]");
	}

	for(int i = 0; i < size; i++)
	{
		gettimeofday(&start, NULL);
		srand(start.tv_usec);
		rand = (double) random() / RAND_MAX;
		
		if(sol_heur[i] > 0.5 && rand < prob)
		{
			CPXchgbds(env, lp, 1, &i, &lb, &one);
		}
	}
}

static int CPXPUBLIC mylazycallback(CPXCENVptr env, void *cbdata, int wherefrom, void *cbhandle, int *useraction_p)
{
	*useraction_p = CPX_CALLBACK_DEFAULT; 
	instance* inst = (instance *) cbhandle; 			// casting of cbhandle    

	// get solution xstar
	int ncols = inst->nnodes * (inst->nnodes - 1) / 2;
	double *xstar = (double*) malloc(ncols * sizeof(double));
	if ( CPXgetcallbacknodex(env, cbdata, wherefrom, xstar, 0, ncols-1) )
	{
		printf("Error in mylazycallback\n");
		return 1; // y = current y from CPLEX-- y starts from position 0
	}	
		
	// get some random information at the node (as an example)
	//----------------- GET USEFUL INFORMATIONS ON THE CURRENT STATE ----------------
	
	double objval = CPX_INFBOUND; 
	CPXgetcallbacknodeobjval(env, cbdata, wherefrom, &objval);   
	int mythread = -1;
	CPXgetcallbackinfo(env, cbdata, wherefrom, CPX_CALLBACK_INFO_MY_THREAD_NUM, &mythread);    
	double zbest;
	CPXgetcallbackinfo(env, cbdata, wherefrom, CPX_CALLBACK_INFO_BEST_INTEGER, &zbest); 
	//printf("i am thread <%d>\n", mythread);

	//apply cut separator and possibly add violated cuts
	int ncuts = myseparation(inst, xstar, env, cbdata, wherefrom);	    
	//printf("cuts is <%d>\n", ncuts);
	free(xstar);
	
	if ( ncuts > 1 )
	{
		*useraction_p = CPX_CALLBACK_SET; 		// tell CPLEX that cuts have been created
	}
	return 0; 
}

void prim_dijkstra_MST(instance *inst, int **pred)
{
	const int n = inst->nnodes;
	int *flag = (int *) calloc(inst->nnodes, sizeof(int));
	double *L = (double *) calloc(inst->nnodes, sizeof(double));
	*pred = (int *) calloc(inst->nnodes, sizeof(int));

	flag[0] = 1;
	(*pred)[0] = 0;

	for(int j = 1; j < n; j++)
	{
		flag[j] = 0;
		L[j] = dist(1, j, inst);
		(*pred)[j] = 0;
	}

	for(int k = 1; k < n; k++)
	{
		double min = DBL_MAX;
		int h;
		for(int j = 1; j < n; j++)
		{
			if(flag[j] == 0 && L[j] < min)
			{
				min = L[j];
				h = j;
			}
		}
		flag[h] = 1;
		for(int j = 1; j < n; j++)
		{
			double c_h_j = dist(h, j, inst);
			if(flag[j] == 0 && c_h_j < L[j])
			{
				L[j] = c_h_j;
				(*pred)[j] = h;
			}
		}
	}

	free(flag);
	free(L);
}

void add_children(struct node *node, int *prev, int *visited, int size)
{
	int idx = -1;
	for(int i = 1; i < size; i++)
	{
		if(prev[i] == node->number && !visited[i])
		{
			if(node->firstchild == NULL)
			{
				node->firstchild = (struct node *) malloc(sizeof(struct node));
				node->firstchild->number = i;
				node->firstchild->firstchild = NULL;
				node->firstchild->nextsibling = NULL;
				add_children(node->firstchild, prev, visited, size);
			}
			else
			{
				struct node *current_node = node->firstchild;
				while(current_node->nextsibling != NULL)
				{
					current_node = current_node->nextsibling;
				}

				current_node->nextsibling = (struct node *) malloc(sizeof(struct node));
				current_node->nextsibling->number = i;
				current_node->nextsibling->firstchild = NULL;
				current_node->nextsibling->nextsibling = NULL;
				add_children(current_node->nextsibling, prev, visited, size);
			}
		}
	}
}

void pre_order_visit(struct node *node, int *tour, int *tour_idx)
{
	if(node == NULL) 
		return;

	if(VERBOSE > 5000)
	{
		printf("Node %d visited\n", (node->number)+1);
	}

	tour[*tour_idx] = node->number;
	(*tour_idx)++;

	pre_order_visit(node->firstchild, tour, tour_idx);
	pre_order_visit(node->nextsibling, tour, tour_idx);
}

void two_approx_algorithm_TSP(instance *inst, int **approx_tour_ptr)
{
	*approx_tour_ptr = (int *) calloc(inst->nnodes, sizeof(int));
	int tour_idx = 0;

	if(!strncmp(inst->dist_type, "GEO", 3)) // not sure if the GEO distance obey the triangle inequality
	{
		if(VERBOSE > 50)
		{
			printf("GEO distance: cannot apply the 2-approximation algorithm for the TSP\n");
			printf("Returning the stupid solution\n");
		}
		for(int i = 0; i < inst->nnodes; i++)
		{
			(*approx_tour_ptr)[i] = i;
		}
		return;
	}

	int *prev = NULL;
	prim_dijkstra_MST(inst, &prev);
	
	if(VERBOSE > 5000)
	{
		for(int i = 1; i < inst->nnodes; i++)
		{
			printf("Edge %d %d\n", (prev[i]+1), (i+1));
		}
	}

	struct node *root = (struct node *) malloc(sizeof(struct node));
	root->number = 0;
	root->firstchild = NULL;
	root->nextsibling = NULL;
	int *visited = (int *) calloc(inst->nnodes, sizeof(int));
	add_children(root, prev, visited, inst->nnodes);

	pre_order_visit(root, *approx_tour_ptr, &tour_idx);

	free(prev);
	free_tree(root);
}


void free_tree(struct node *node)
{
	if(node == NULL)
		return;
	
	free_tree(node->nextsibling);
	free_tree(node->firstchild);
		
	free(node);
}