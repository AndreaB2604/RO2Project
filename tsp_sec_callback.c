#include "tsp.h"

static int CPXPUBLIC mylazycallback(CPXCENVptr env, void *cbdata, int wherefrom, void *cbhandle, int *useraction_p);

int TSPopt_sec_callback(instance *inst)
{
	// open cplex model
	int error;
	int cur_numcols;
	double obj_val;

	CPXENVptr env = CPXopenCPLEX(&error);
	// CPXsetintparam(env, CPXPARAM_Read_DataCheck, 1);			// used to check if there are errors while reading data

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
	CPXsetintparam(env, CPXPARAM_RandomSeed, inst->random_seed);
	double detstart, detend, exec_det_time;
	unsigned long start, exec_time;
	if(dettime)
	{
		CPXsetdblparam(env, CPXPARAM_DetTimeLimit, 2127600.0);
	}
	else
	{
		CPXsetdblparam(env, CPXPARAM_TimeLimit, 3600.0);
	}

	if(CPXgetdettime(env, &detstart))
	{
		print_error("Error in getting deterministic time start");
	}
	
	// Solve the problem
	start = microseconds();
	if(CPXmipopt(env, lp))
	{
		print_error("Optimisation failed in TSPopt_sec_loop()");
	}
	exec_time = microseconds() - start;

	if(CPXgetdettime(env, &detend))
	{
		print_error("Error in getting deterministic time end");
	}
	exec_det_time = detend - detstart;


	// Retrieve the best solution and put it in the instance
	cur_numcols = inst->nnodes * (inst->nnodes - 1) / 2;
	inst->best_sol = (double *) calloc(cur_numcols, sizeof(double));
	if(CPXgetx(env, lp, inst->best_sol, 0, cur_numcols-1))
	{
		print_error("Failure to read the optimal solution in CPXgetx()");
	}

	// save model
	if(VERBOSE >= 100)
	{
		CPXwriteprob(env, lp, "tsp_sec_loop.lp", NULL); 
	}

	// print only the non-zero variables
	if(VERBOSE > 100)
	{
		for(int k = 0; k < cur_numcols; k++)
		{
			if(inst->best_sol[k] > TOLERANCE)
			{	
				int l = inst->nnodes -1;
				int flag = 0;
				for(int i=0; (i<inst->nnodes-1) && (!flag); i++)
				{
					if(k<l)
					{
						for(int j=i+1; j<inst->nnodes; j++)
						{
							if(xpos(i, j, inst) == k) 
							{
								printf("x_%d_%d = %f\n", i+1, j+1, inst->best_sol[k]);
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
	}
	int lpstat = CPXgetstat(env,lp);
	int solved = (lpstat == CPXMIP_OPTIMAL) || (lpstat == CPXMIP_OPTIMAL_INFEAS) || (lpstat == CPXMIP_OPTIMAL_TOL);
	printf("Solved %d\n", solved);
	printf("Execution dettime of sec_callback = %.3f ticks\n", exec_det_time);
	printf("Execution time of sec_callback = %.3f s\n", ((double)exec_time/1000000));

	// get the best solution and print it
	if(CPXgetobjval(env, lp, &obj_val))
	{
		print_error("Failure to read the value of the optimal solution in CPXgetobjval()");
	}
	printf("\nSolution value  = %lf\n", obj_val);

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
		free(xstar);
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

int myseparation(instance *inst, double *xstar, CPXCENVptr env, void *cbdata, int wherefrom)
{
	int num_comp;
	int comp[inst->nnodes];
	int conn_comp[inst->nnodes];

	int cur_numcols = inst->nnodes * (inst->nnodes - 1) / 2;  

	connected_components(inst, xstar, cur_numcols, comp, conn_comp, &num_comp);

	if(VERBOSE > 1000)
	{
		// print the component for each node
		for(int i = 0; i < inst->nnodes; i++)
		{
			printf("comp[%d] = %d\n", (i+1), (comp[i]+1));
		}
	}

	if(VERBOSE > 100)
	{
		for(int i = 0; i < num_comp; i++)
		{
			printf("conn_comp[%d] = %d\n", i, (conn_comp[i]+1));
		}
		printf("Number of connected components = %d\n", num_comp);
	}

	if(num_comp == 1)
	{
		return 0;
	}

	// I have to add the SECs
	for(int k = 0; k < num_comp; k++)
	{
		int num_x_var = (inst->nnodes-1) * inst->nnodes / 2; 	// number of x variables
		int nzcnt = 0;					// number of non-zero variables in the constraint
		double rhs = -1.0;
		char sense = 'L';
		double rmatval[num_x_var];		// coefficients of the non-zero variables
		int rmatind[num_x_var]; 		// position of the variables to set (in terms of columns)

		for(int i = 0; i < num_x_var; i++)
		{
			rmatval[i] = 0;
		}

		for(int i = 0; i < inst->nnodes; i++)
		{
			if(comp[i] == conn_comp[k])
			{
				rhs += 1.0;
				for(int j = i+1; j < inst->nnodes; j++)
				{
					if(comp[j] == conn_comp[k])
					{
						rmatind[nzcnt] = xpos(i, j, inst);
						rmatval[nzcnt] = 1.0;
						nzcnt++;
					}
				}
			}
		}

		if(CPXcutcallbackadd(env, cbdata, wherefrom, nzcnt, rhs, sense, rmatind, rmatval, 0))
		{
			print_error("USER_separation: CPXcutcallbackadd error");
		}
		
	}

	return num_comp;
}
