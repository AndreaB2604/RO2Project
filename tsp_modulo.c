#include "tsp.h"

static int CPXPUBLIC mylazycallback(CPXCENVptr env, void *cbdata, int wherefrom, void *cbhandle, int *useraction_p);

int TSP_modulo(instance *inst)
{
	double time_limit = inst->time_limit;

	inst->time_limit = time_limit * 2 / 3;

	if(TSP_heur_vns(inst))
		print_error(" error within TSP_heur_vns()");

	inst->time_limit = time_limit / 3;


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

	double *values = (double *) calloc(inst->nnodes, sizeof(double)); // values of the variables (1.0)
	int *varindices = (int *) calloc(inst->nnodes, sizeof(double)); // indexes of the variables to change
	
	for(int i = 0; i < inst->nnodes; i++)
	{
		values[i] = 1.0;
	}

	int cur_numcols = inst->nnodes * (inst->nnodes - 1) / 2;
	double best_obj_val = 0;
	for(int k = 0, a = 0; k < cur_numcols; k++)
	{
		if(inst->best_sol[k] > TOLERANCE)
		{
			varindices[a++] = k;
			int l = inst->nnodes -1;
			int flag = 0;
			for(int i = 0; (i<inst->nnodes-1) && (!(flag)); i++)
			{
				if(k<l)
				{
					for(int j=i+1; j<inst->nnodes; j++)
					{
						if(xpos(i, j, inst) == k) 
						{
							best_obj_val += dist(i, j, inst);
							flag = 1;
							break;
						}
					}
				}
				else
				{
					l += inst->nnodes-i-2; 
				}
			}	
		}
		
		
	}

	//----------- for BLADE purposes-------------//
	int flag[3] = {0};
	int limit = inst->time_limit;
	int counter = 0;
	if(BLADE)
	{
		printf("solution value at %d is: %f\n", counter, best_obj_val);
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

	int idx_prob_rate = 0;
	double callback_time_limit = 10;	// time-limit of the callback
	unsigned long start = microseconds();
	while(!done)
	{
		// save model
		unsigned long current_time = microseconds();
		double elapsed = (double) (current_time - start) / 1000000;
		if(BLADE)
		{
			for(int a=0; a<3; ++a)
			{
				double ub = (double) ((a+1)*limit/3);
				if((!flag[a]) && (elapsed >= ub))
				{
					printf("solution value at %d is: %f\n", (a+1), best_obj_val);
					flag[a] = 1;
					break;
				}
			}
		}
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
			else
			{
				idx_prob_rate = (idx_prob_rate == 0)? idx_prob_rate : idx_prob_rate-1;
			}

			callback_time_limit = (elapsed+callback_time_limit <= inst->time_limit)? callback_time_limit : inst->time_limit-elapsed;
			CPXsetdblparam(env, CPXPARAM_TimeLimit, callback_time_limit);
			random_fixing(env, lp, inst->best_sol, cur_numcols, prob_rate[idx_prob_rate]);
			
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
			if(VERBOSE >= 5000)
			{
				CPXwriteprob(env, lp, "tsp_hard_fixing.lp", NULL); 
			}
		}
	}

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

	//apply cut separator and possibly add violated cuts
	int ncuts = myseparation(inst, xstar, env, cbdata, wherefrom);	    
	free(xstar);
	
	if ( ncuts > 1 )
	{
		*useraction_p = CPX_CALLBACK_SET; 		// tell CPLEX that cuts have been created
	}
	return 0; 
}