#include "tsp.h"

static int CPXPUBLIC mylazycallback(CPXCENVptr env, void *cbdata, int wherefrom, void *cbhandle, int *useraction_p);

int TSP_heur_lb(instance *inst)
{
	double lb_rate[] = {3, 5, 10};
	int lb_rate_max_idx = 2;
	// open cplex model
	int error, status;
	int done = 0;

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
	int idx_lb_rate = 0;
	double callback_time_limit = 10;	// time-limit of the callback
	unsigned long start = microseconds();
	char **cname = (char **) calloc(1, sizeof(char *));		// (char **) required by cplex...
	cname[0] = "Local Branching constraint";
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
				idx_lb_rate = (idx_lb_rate == lb_rate_max_idx)? idx_lb_rate : idx_lb_rate+1;
			}
			else
			{
				idx_lb_rate = (idx_lb_rate == 0)? idx_lb_rate : idx_lb_rate-1;
			}
			if(VERBOSE >= 500)
			{
				printf("rate is : %f\n", lb_rate[idx_lb_rate]);
			}

			callback_time_limit = (elapsed+callback_time_limit <= inst->time_limit)? callback_time_limit : inst->time_limit-elapsed;
			CPXsetdblparam(env, CPXPARAM_TimeLimit, callback_time_limit);

			// adding the local branching constraint
			int cur_numcols = inst->nnodes * (inst->nnodes - 1) / 2;
			int j = 0;
			double rhs = (double) inst->nnodes - lb_rate[idx_lb_rate];
			char sense = 'G';
			double rmatval[inst->nnodes];	// coefficients of the non-zero variables
			int rmatind[inst->nnodes]; 		// position of the variables to set (in terms of columns)
			int rmatbeg = 0;				// start positions of the constraint

			for(int i = 0; i < inst->nnodes; i++)
			{
				rmatval[i] = 1.0;
			}
			for(int i = 0; i < cur_numcols; i++)
			{
				if(inst->best_sol[i] > TOLERANCE)
				{
					rmatind[j++] = i;
				}
			}
			if(CPXaddrows(env, lp, 0, 1, inst->nnodes, &rhs, &sense, &rmatbeg, rmatind, rmatval, NULL, cname))
			{
				print_error(" wrong local branching constraint added");
			}

			if((status = CPXmipopt(env, lp)))
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
				if(VERBOSE >= 100)
				{
					printf("Best x_heu: %f\nIncumbent solution: %f\n\n", best_obj_val, current_obj_val);
				}

				best_obj_val = current_obj_val;
				counter = 0;
				idx_lb_rate = 0;

				if(CPXgetx(env, lp, inst->best_sol, 0, cur_numcols-1))
				{
					print_error("Failure to read the optimal solution in CPXgetx()");
				}
			}
			else
			{
				counter++;
			}

			// remove the local branching constraint
			if(CPXdelrows(env, lp, CPXgetnumrows(env, lp)-1, CPXgetnumrows(env, lp)-1))
			{
				print_error(" wrong local branching constraint removed");
			}

			// save model
			if(VERBOSE >= 5000)
			{
				CPXwriteprob(env, lp, "tsp_local_branching.lp", NULL);
			}
		}
	}

	free(cname);
	free(approx_tour);
	free(values);
	free(varindices);

	// Free up the problem as allocated by CPXcreateprob, if necessary
	if(lp != NULL)
	{
		CPXfreeprob(env, &lp);
	}

	// Free up the CPLEX environment, if necessary
	if(env != NULL) 
	{
		CPXcloseCPLEX(&env);
	}

	return 0;

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
