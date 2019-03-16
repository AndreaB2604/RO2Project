#include "tsp.h"

void build_model_compact(instance *inst, CPXENVptr env, CPXLPptr lp)
{
	int M = inst->nnodes;
	double obj;
	double lb, ub;				// lower bound and upper bound
	char binary = 'B';
	char general = 'I';

	char **cname = (char **) calloc(1, sizeof(char *));		// (char **) required by cplex...
	cname[0] = (char *) calloc(100, sizeof(char));

	// Adding x_i_j variables
	for(int i=0; i<inst->nnodes; i++)
	{
		for(int j=0; j<inst->nnodes; j++)
		{
			sprintf(cname[0], "x_%d_%d", i+1,j+1);
			obj = (i==j)? 0.0 : dist(i,j,inst);
			lb = 0.0;
			ub = (i==j)? 0.0 : 1.0;
			if(VERBOSE > 1000)
			{
				printf("The variable %s number %d has value %lf\n", cname[0], xpos_compact(i,j,inst), obj);
			}
			if(CPXnewcols(env, lp, 1, &obj, &lb, &ub, &binary, cname)) 
			{
				print_error(" wrong CPXnewcols on x var.s");
			}
			if(CPXgetnumcols(env,lp)-1 != xpos_compact(i,j, inst))
			{
				print_error(" wrong position for x var.s");
			}
		}
	}

	// Adding u_i variables
	for(int i=0; i<inst->nnodes; i++)
	{
		sprintf(cname[0], "u_%d", i+1);
		obj = 0.0; 		// since the u_i variables don't have to appear in the objective function
		lb = (i==0)? 1.0 : 2.0;  // as from the article MTZ
		ub = (i==0)? 1.0 : inst->nnodes;
		if(CPXnewcols(env, lp, 1, &obj, &lb, &ub, &general, cname)) 
		{
			print_error(" wrong CPXnewcols on u var.s");
		}
		/*
		if(CPXgetnumcols(env,lp)-1 != (xpos_compact(inst->nnodes-1, inst->nnodes-1, inst) + i + 1))
		{
			print_error(" wrong position for x var.s");
		}
		*/
	}
	
	// Adding in_degree constraints (summation over i of x_i_h = 1)
	for(int h=0; h<inst->nnodes; h++)
	{
		int lastrow = CPXgetnumrows(env, lp);
		double rhs = 1.0;
		char sense = 'E';
		sprintf(cname[0], "indeg(%d)", h+1);
		if(CPXnewrows(env, lp, 1, &rhs, &sense, NULL, cname)) 
		{
			print_error(" wrong CPXnewrows [x1]");
		}
		for(int i=0; i < inst->nnodes; i++)
		{
			if(i == h) { continue; }
			else
			{
				if(CPXchgcoef(env, lp, lastrow, xpos_compact(i, h, inst), 1.0)) 
				{
					print_error(" wrong CPXchgcoef [x1]");
				}
			}
		}
	}

	// Adding out degree constraints (summation over j of x_h_j = 1)
	for(int h=0; h<inst->nnodes; h++)
	{
		int lastrow = CPXgetnumrows(env, lp);
		double rhs = 1.0;
		char sense = 'E';
		sprintf(cname[0], "outdeg(%d)", h+1);
		if(CPXnewrows(env, lp, 1, &rhs, &sense, NULL, cname)) 
		{
			print_error(" wrong CPXnewrows [x1]");
		}
		for(int j=0; j < inst->nnodes; j++)
		{
			if(j == h) { continue; }
			else
			{
				if(CPXchgcoef(env, lp, lastrow, xpos_compact(h, j, inst), 1.0)) 
				{
					print_error(" wrong CPXchgcoef [x1]");
				}
			}
		}
	}

	// Adding lazy constraints ( x_i_j + x_j_i <= 1 )
	for(int i=0; i<inst->nnodes; i++)
	{
		for(int j=i+1; j<inst->nnodes; j++)
		{
			double rhs = 1.0;			// right hand side
			char sense = 'L';
			int rcnt = 1;				// number of lazy constraint to add
			int nzcnt = 2;				// number of non-zero variables in the constraint
			double rmatval[] = {1.0, 1.0};		// coefficient of the non-zero variables
			// position of the variables to set (in terms of columns)
			int rmatind[] = { xpos_compact(i,j,inst), xpos_compact(j,i,inst) };
			//int rmatbeg[] = { 0, 2 };
			int rmatbeg = 0;			// start positions of the constraint
			sprintf(cname[0], "lazy_const_x(%d,%d)", i+1, j+1);
			if(CPXaddlazyconstraints(env, lp, rcnt, nzcnt, &rhs, &sense, &rmatbeg, rmatind, rmatval, cname)) 
			{
				print_error(" wrong lazy constraint x_i_j + x_j_i <= 1");
			}
		}
	}

	// Adding big-M lazy constraints ( M*x_i_j + u_i - u_j <= M-1 ) 
	for(int i=0; i<inst->nnodes; i++)
	{
			
		if(i==0)
		{
			double rhs = 1.0;
			char sense = 'E';
			int rcnt = 1;
			int nzcnt = 1;
			double rmatval = 1.0;
			int rmatind = xpos_compact(inst->nnodes-1, inst ->nnodes-1, inst)+1;
			int rmatbeg = 0;
			sprintf(cname[0], "lazy_cost(u_1)");
			if(CPXaddlazyconstraints(env, lp, rcnt, nzcnt, &rhs, &sense, &rmatbeg, &rmatind, &rmatval, cname)) 
			{
				print_error(" wrong lazy [u1]");
			}

		} 
		else
		{ 
			for(int j=1; j<inst->nnodes; j++)
			{
				if(i==j) { continue; }
				int num_x_var = inst->nnodes * inst->nnodes; 	// == xpos_compact(inst->nnodes-1, inst->nnodes-1, inst) + 1
				double rhs = (double) M - 1.0;					// right hand side
				char sense = 'L';
				int rcnt = 1;									// number of lazy constraint to add
				int nzcnt = 3;									// number of non-zero variables in the constraint
				double rmatval[] = {1.0, -1.0, (double) M};		// coefficient of the non-zero variables
				int rmatind[] = {num_x_var+i, num_x_var+j, xpos_compact(i,j,inst)};
				int rmatbeg = 0;								// start positions of the constraint
				sprintf(cname[0], "lazy_const_u(%d,%d)", i+1, j+1);
				if(CPXaddlazyconstraints(env, lp, rcnt, nzcnt, &rhs, &sense, &rmatbeg, rmatind, rmatval, cname)) 
				{
					print_error(" wrong lazy M*x_i_j + u_i - u_j <= M-1");
				}
			}
		}
	}

	int max = xpos_compact(inst->nnodes-1, inst->nnodes-1, inst);
	printf("\n\nThe number of variables is %d\n", max);

	// save model
	if(VERBOSE >= 100)
	{
		CPXwriteprob(env, lp, "tsp_compact.lp", NULL);
	}
}


int TSPopt_compact(instance *inst)
{
	
	//inst->tstart = second(); maybe we'll need it  
	//inst->best_lb = -CPX_INFBOUND;   

	// open cplex model
	int i, j, k, l, flag;
	int error, status;
	int cur_numrows, cur_numcols;
	double obj_val;

	CPXENVptr env = CPXopenCPLEX(&error);
	// CPXsetintparam(env, CPXPARAM_Read_DataCheck, 1);			// used to check if there are errors while reading data

	CPXLPptr lp = CPXcreateprob(env, &error, "TSP_COMPACT");
	CPXsetlogfilename(env, "exec_compact_log.txt", "w");			// it saves the log of the computation in exec_compact_log.txt

	// build model
	build_model_compact(inst, env, lp);

	// solve the optimisation problem
	if(CPXmipopt(env, lp))
	{
		print_error("Optimisation failed in TSPopt_compact()");
	}

	// cur_numrows is the number of nodes == inst->nnodes
	// cur_numcols is the number of variables 
	cur_numrows = CPXgetnumrows(env, lp);
	cur_numcols = CPXgetnumcols(env, lp);
	int max_idx_x = inst->nnodes * inst->nnodes; 	// == xpos_compact(inst->nnodes-1, inst->nnodes-1, inst)
	
	// get the optimal solution of the variables
	inst->best_sol = (double *) calloc(cur_numcols, sizeof(double));
	if(CPXgetx(env, lp, inst->best_sol, 0, cur_numcols-1))
	{
		print_error("Optimisation failed in TSPopt()");
	}

	// print only the non-zero variables
	if(VERBOSE > 50)
	{
		// print the x variables that are non-zero
		for(k = 0; k < max_idx_x; k++)
		{
			for(i=0; i<inst->nnodes; i++)
			{
				for(j=0; j<inst->nnodes; j++)
				{
					if((i!=j) && (xpos_compact(i, j, inst) == k) && (inst->best_sol[k] > TOLERANCE)) 
					{
						printf("x_%d_%d = %f\n", i+1, j+1, inst->best_sol[k]);
					}
				}
			}
		}

		// print the u variables that are non-zero
		for(k=0; k<inst->nnodes; k++)
		{
			printf("u_%d = %f\n", k+1, inst->best_sol[k+max_idx_x]);
		}

	}
	//printf("cur_numcols = %d\n", xpos(inst->nnodes-2, inst->nnodes-1, inst));
	//printf("cur_numcols = %d\n", cur_numcols);
	
	// get the best solution and print it
	if(CPXgetobjval(env, lp, &obj_val))
	{
		print_error("Optimisation failed in TSPopt_compact()");
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

int xpos_compact(int i, int j, instance *inst)
{
	if((i >= inst->nnodes) || (j >= inst->nnodes) || (i<0) || (j<0))
	{
		print_error("Domain contraint not respected");
	}
	return (i*inst->nnodes + j);
}
