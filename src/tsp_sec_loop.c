#include "tsp.h"

double ticks[] = {5000.0, 10000.0, 30000.0, 45000.0, 60000.0, 1.0e+75};
int rins_nodes[] = {0, 100, 45, 10, 5};
//int rins_nodes[] = {0, 0, 0, 0, 0};
int max_ticks_idx = 5;
int max_rins_idx = 4;
int rins_active = 1; // 0 rins not active, 1 otherwise
const double HOUR_TICKS = 2127600.0;

// add the SECs
int sec_loop(CPXENVptr env, CPXLPptr lp, instance *inst)
{
	int num_comp;
	int comp[inst->nnodes];
	int conn_comp[inst->nnodes];

	int cur_numcols = CPXgetnumcols(env, lp);

	connected_components(inst, inst->best_sol, cur_numcols, comp, conn_comp, &num_comp);

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
		int rmatbeg = 0;				// start positions of the constraint

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

		if(CPXaddrows(env, lp, 0, 1, nzcnt, &rhs, &sense, &rmatbeg, rmatind, rmatval, NULL, NULL))
		{
			print_error(" wrong SEC constraint");
		}
	}
	return 1;
}


int TSPopt_sec_loop(instance *inst)
{
	// open cplex model
	const double GAP_TOLERANCE = 1e-04;		// the default value in CPLEX
	int error, status;
	int done = 0;
	int cur_numcols;
	double obj_val;

	CPXENVptr env = CPXopenCPLEX(&error);
	// CPXsetintparam(env, CPXPARAM_Read_DataCheck, 1);			// used to check if there are errors while reading data
	CPXsetintparam(env, CPXPARAM_RandomSeed, inst->random_seed);
	CPXLPptr lp = CPXcreateprob(env, &error, "TSP"); 
	if(VERBOSE > 50)
	{
		CPXsetlogfilename(env, "exec_log.txt", "w");			// it saves the log of the computation in exec_compact_log.txt
	}

	// build model
	build_model(inst, env, lp);

	cur_numcols = CPXgetnumcols(env, lp);
	inst->best_sol = (double *) calloc(cur_numcols, sizeof(double));	

	int rins_idx = 0;
	int timestamp_idx = 0;
	unsigned long start = microseconds();
	unsigned long elapsed = 0;
	double detstart;
	double detelapsed = 0;
	if(CPXgetdettime(env, &detstart))
	{
		print_error("Error in getting deterministic time start");
	}
	while(!done)
	{
		CPXsetdblparam(env, CPXPARAM_DetTimeLimit, ticks[timestamp_idx]);
		if(rins_active)
		{
			CPXsetintparam(env, CPXPARAM_MIP_Strategy_RINSHeur, rins_nodes[rins_idx]); 
		}
		if(dettime)
		{
			if(CPXgetdettime(env, &detelapsed))
			{
					print_error("Error in getting deterministic time elapsed");
			}
			if((detelapsed - detstart) >= HOUR_TICKS)
			{
					break;
			}
		}
		else
		{
			elapsed = microseconds() - start;
			if(((double)elapsed/1000000) >= 3600.0)
			{
					break;
			}
		}

		// solve the optimisation problem
		if(CPXmipopt(env, lp))
		{
			print_error("Optimisation failed in TSPopt_sec_loop()");
		}
		
		// get the optimal solution of the variables
		if(CPXgetx(env, lp, inst->best_sol, 0, cur_numcols-1))
		{
			print_error("Failure to read the optimal solution in CPXgetx()");
		}

		// check the relative gap
		double gap;
		status = CPXgetmiprelgap(env, lp, &gap);
		if(status)
		{
			print_error("Error in getting the gap in CPXgetmiprelgap()");
		}

		// check if have to add (new) SECs
		char res_conn_comp = sec_loop(env, lp, inst);
		if(VERBOSE > 100)
		{
			printf("Gap = %.5f%%\n", (gap*100));
			printf("Timelimit = %f ticks\n", ticks[timestamp_idx]);
			printf("Rins = %d\n\n", rins_nodes[rins_idx]);
		}
		
		if(!res_conn_comp)
		{
			int lpstat = CPXgetstat(env,lp);
			int solved = (lpstat == CPXMIP_OPTIMAL) || (lpstat == CPXMIP_OPTIMAL_INFEAS) || (lpstat == CPXMIP_OPTIMAL_TOL);
			if(solved)
			{
				done = 1;
			}
			else
			{
				timestamp_idx++;
				if(rins_active)
				{
					rins_idx = 0;
				}
			}
		}
		else
		{
			if(gap < GAP_TOLERANCE)
			{

				if(rins_active)
				{
					if(timestamp_idx <= ((max_ticks_idx+1)/2))
					{
						rins_idx = (rins_idx == 0)? rins_idx : (rins_idx-1) ; // Decrementing the RINS verbosity	
					}
					else
					{
						rins_idx = (rins_idx == max_rins_idx)? rins_idx : (rins_idx+1) ; // Incrementing the RINS verbosity	
					}
				}
				timestamp_idx = (timestamp_idx == 0)? timestamp_idx : (timestamp_idx-1);
			}
			else
			{
				
				if(rins_active)
				{
					if(timestamp_idx <= ((max_ticks_idx+1)/2))
					{
						rins_idx = 0; // setting RINS to default
					}
					else
					{
						rins_idx = (rins_idx == max_rins_idx)? rins_idx : (rins_idx+1) ; // Incrementing the RINS verbosity	
					}
				}
				timestamp_idx = (timestamp_idx == max_ticks_idx)? timestamp_idx : (timestamp_idx+1);
			}
		}
	}
	unsigned long exec_time = microseconds() - start;

	double detend;
	if(CPXgetdettime(env, &detend))
	{
			print_error("Error in getting deterministic time end");
	}
	double exec_det_time = detend - detstart;

	// save model
	if(VERBOSE >= 100)
	{
		CPXwriteprob(env, lp, "tsp_sec_loop.lp", NULL); 
	}

	// print only the non-zero variables
	if(VERBOSE > 50)
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
	printf("Execution dettime of sec_loop = %.3f ticks\n", exec_det_time);
	printf("Execution time of sec_loop = %.3f s\n", ((double)exec_time/1000000));

	// get the best solution and print it
	if(CPXgetobjval(env, lp, &obj_val))
	{
		print_error("Failure to read the value of the optimal solution in CPXgetobjval()");
	}
	printf("\nSolution value  = %lf\n", obj_val);

	// Free up the problem as allocated by CPXcreateprob, if necessary
	if(lp != NULL)
	{
		status = CPXfreeprob(env, &lp);
	}

	// Free up the CPLEX environment, if necessary
	if(env != NULL) 
	{
		status = CPXcloseCPLEX(&env);
	}

	return 0;
}
