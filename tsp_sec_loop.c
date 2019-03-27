#include "tsp.h"

// computes the connected components and add the SECs
int sec_loop(CPXENVptr env, CPXLPptr lp, instance *inst)
{
	int comp[inst->nnodes];
	int cur_numcols = CPXgetnumcols(env, lp);
	for(int i = 0; i < inst->nnodes; i++)
	{
		comp[i] = i;		// components go from 0 to n-1
	}

	for(int k = 0; k < cur_numcols; k++)
	{	
		int l = inst->nnodes-1;
		int flag = 0;
		for(int i=0; (i<inst->nnodes-1) && (!flag); i++)
		{
			if(k<l)
			{
				for(int j=i+1; j<inst->nnodes; j++)
				{
					// if the {i,j} exists then i and j belong to the same connected component
					if((xpos(i, j, inst) == k) && (inst->best_sol[k] > TOLERANCE)) 
					{
						if(comp[i] != comp[j])
						{
							int ci = comp[i];
							int cj = comp[j];
							for(int n = 0; n < inst->nnodes; n++)
							{
								if(comp[n] == cj)
								{
									comp[n] = ci;
								}
							}
						}	
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

	if(VERBOSE > 1000)
	{
		// print the component for each node
		for(int i = 0; i < inst->nnodes; i++)
		{
			printf("comp[%d] = %d\n", (i+1), (comp[i]+1));
		}
	}

	int conn_comp[inst->nnodes]; 			// this array will contain all the value of the connected components
	for(int i = 0; i < inst->nnodes; i++)
	{
		conn_comp[i] = -1;
	}

	int num_comp = 0; // number of current connected components
	// compute what the connected components are and their number
	for(int i = 0; i < inst->nnodes; i++)
	{
		for(int j = 0; (j < inst->nnodes) && (comp[i] != conn_comp[j]); j++)
		{
			if(conn_comp[j] == -1)
			{
				conn_comp[j] = comp[i];
				num_comp++;
				break;
			}
		}
	}

	if(VERBOSE > 1000)
	{
		for(int i = 0; i < num_comp; i++)
		{
			printf("conn_comp[%d] = %d\n", i, (conn_comp[i]+1));
		}
		printf("Number of connected components = %d\n", num_comp);
	}

	if(num_comp == 1)
	{
		if(VERBOSE > 1000)
		{
			printf("Only one connected component remained, problem solved\n");
		}
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
	//inst->tstart = second(); maybe we'll need it  
	//inst->best_lb = -CPX_INFBOUND;   

	// open cplex model
	int error, status;
	int done = 0;
	int cur_numrows, cur_numcols;
	double obj_val;

	CPXENVptr env = CPXopenCPLEX(&error);
	CPXsetintparam(env, CPXPARAM_Read_DataCheck, 1);			// used to check if there are errors while reading data
	CPXLPptr lp = CPXcreateprob(env, &error, "TSP"); 
	CPXsetlogfilename(env, "exec_log.txt", "w");			// it saves the log of the computation in exec_compact_log.txt

	// build model
	build_model(inst, env, lp);

	while(!done)
	{
		// solve the optimisation problem
		if(CPXmipopt(env, lp))
		{
			print_error("Optimisation failed in TSPopt_sec_loop()");
		}

		cur_numrows = CPXgetnumrows(env, lp);
		cur_numcols = CPXgetnumcols(env, lp);
		
		// get the optimal solution of the variables
		inst->best_sol = (double *) calloc(cur_numcols, sizeof(double));
		if(CPXgetx(env, lp, inst->best_sol, 0, cur_numcols-1))
		{
			print_error("Failure to read the optimal solution in CPXgetx()");
		}

		// check if have to add (new) SECs
		if(!sec_loop(env, lp, inst))
		{
			done = 1;
		}
	}

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
			int l = inst->nnodes -1;
			int flag = 0;
			for(int i=0; (i<inst->nnodes-1) && (!flag); i++)
			{
				if(k<l)
				{
					for(int j=i+1; j<inst->nnodes; j++)
					{
						if((xpos(i, j, inst) == k) && (inst->best_sol[k] > TOLERANCE)) 
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
