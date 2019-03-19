#include "tsp.h" 

void build_model_compact_custom(instance *inst, CPXENVptr env, CPXLPptr lp)
{
	char binary = 'B';

	char **cname = (char **) calloc(1, sizeof(char *));		// (char **) required by cplex...
	cname[0] = (char *) calloc(100, sizeof(char));

	// Adding variables x_i_j to the model + their respective cost in the objective function
	for(int i = 0; i < inst->nnodes; i++)
	{
		for(int j = i+1; j < inst->nnodes; j++ )
		{
			sprintf(cname[0], "x_%d_%d", i+1,j+1);
			double obj = dist(i,j,inst);
			double lb = 0.0;  
			double ub = 1.0; // recall we deal with {0,1} variables

			if(VERBOSE > 1000)
			{
				printf("The variable %s number %d has value %lf\n", cname[0], xpos(i,j,inst), obj);
			}
			if(CPXnewcols(env, lp, 1, &obj, &lb, &ub, &binary, cname)) 
			{
				print_error(" wrong CPXnewcols on x var.s");
			}
			if(CPXgetnumcols(env,lp)-1 != xpos(i,j, inst))
			{
				print_error(" wrong position for x var.s");
			}
		}
	}

	// Adding variables z_v_h to the model + their respective cost in the objective function
	for(int v = 0; v < inst->nnodes; v++)
	{
		for(int h = 0; h < inst->nnodes; h++)
		{
			sprintf(cname[0], "z_%d_%d", v+1,h+1);
			double obj = 0.0;
			double lb = (h!=0 || v!=0)? 0.0 : 1.0;  
			double ub = 1.0; // recall we deal with {0,1} variables
			if(VERBOSE > 1000)
			{
				printf("The variable %s number %d has value %lf\n", cname[0], zpos_compact_custom(v,h,inst), obj);
			}
			if(CPXnewcols(env, lp, 1, &obj, &lb, &ub, &binary, cname)) 
			{
				print_error(" wrong CPXnewcols on x var.s");
			}
		}
	}

	// Adding the constraint that each node has to have 2 adjacent edges selected
	for(int h = 0; h < inst->nnodes; h++)  
	{
		int lastrow = CPXgetnumrows(env, lp);
		double rhs = 2.0;
		char sense = 'E';
		sprintf(cname[0], "deg(%d)", h+1);
		if(CPXnewrows(env, lp, 1, &rhs, &sense, NULL, cname)) 
		{
			print_error(" wrong CPXnewrows [x1]");
		}
		for(int i = 0; i < inst->nnodes; i++)
		{
			if(i == h) { continue; }
			if(CPXchgcoef(env, lp, lastrow, xpos(h, i, inst), 1.0)) 
			{
				print_error(" wrong CPXchgcoef [z1]");
			}
		}
	}

	// Adding the constraint that each node has only one position 
	// sum over h of z_v_h = 1 for every v != 1
	for(int v = 1; v < inst->nnodes; v++)
	{
		int lastrow = CPXgetnumrows(env, lp);
		double rhs = 1.0;
		char sense = 'E';
		sprintf(cname[0], "position_constraint_v(%d)", v+1);
		if(CPXnewrows(env, lp, 1, &rhs, &sense, NULL, cname)) 
		{
			print_error(" wrong CPXnewrows [x1]");
		}
		for(int h = 1; h < inst->nnodes; h++)
		{
			if(CPXchgcoef(env, lp, lastrow, zpos_compact_custom(v, h, inst), 1.0)) 
			{
				print_error(" wrong CPXchgcoef [z1]");
			}
		}
	}

	// Adding the constraint that each position is associated with only one node 
	// sum over v of z_v_h = 1 for every h != 0
	for(int h = 1; h < inst->nnodes; h++)
	{
		int lastrow = CPXgetnumrows(env, lp);
		double rhs = 1.0;
		char sense = 'E';
		sprintf(cname[0], "assignment_constraint_h(%d)", h+1);
		if(CPXnewrows(env, lp, 1, &rhs, &sense, NULL, cname)) 
		{
			print_error(" wrong CPXnewrows [x1]");
		}
		for(int v = 1; v < inst->nnodes; v++)
		{
			if(CPXchgcoef(env, lp, lastrow, zpos_compact_custom(v, h, inst), 1.0)) 
			{
				print_error(" wrong CPXchgcoef [z1]");
			}
		}
	}

	// Adding the constraints for the node 1
	// sum over t of z_i_t + x_i_1 <= 1 for every i != 0
	for(int i = 1; i < inst->nnodes; i++)
	{
		int lastrow = CPXgetnumrows(env, lp);
		double rhs = 1.0;
		char sense = 'L';
		sprintf(cname[0], "first_node_constraint_i(%d)", i+1);
		if(CPXnewrows(env, lp, 1, &rhs, &sense, NULL, cname)) 
		{
			print_error(" wrong CPXnewrows [x1]");
		}
		// change of the x_i_1 coefficient
		if(CPXchgcoef(env, lp, lastrow, xpos(i, 0, inst), 1.0)) 
		{
			print_error(" wrong CPXchgcoef [z1]");
		}
		// change of the z_i_t coefficients
		for(int t = 2; t < inst->nnodes-1; t++)
		{
			if(CPXchgcoef(env, lp, lastrow, zpos_compact_custom(i, t, inst), 1.0)) 
			{
				print_error(" wrong CPXchgcoef [z1]");
			}
		}
	}
	
	// Adding the subtour elimination constraints as lazy constraints
	// sum over t of z_i_t + x_i_j + sum over t of z_j_t <= 2 for every i != 0
	for(int i = 1; i < inst->nnodes; i++)
	{
		for(int j = 1; j < inst->nnodes; j++)
		{
			if(i == j) { continue; }
			for(int h = 1; h < inst->nnodes; h++)
			{
				double rhs = 2.0;				// right hand side
				char sense = 'L';
				int rcnt = 1;					// number of lazy constraint to add
				int nzcnt = inst->nnodes - 1;	// number of non-zero variables in the constraint
				double rmatval[inst->nnodes - 1];		// coefficient of the non-zero variables
				for(int t = 0; t < inst->nnodes-1; t++)
				{
					rmatval[t] = 1.0;
				}

				// position of the variables to set (in terms of columns)
				int rmatind[inst->nnodes - 1];
				rmatind[0] = xpos(i, j, inst);
				for(int t = 0; t < h-1; t++)
				{
					rmatind[t+1] = zpos_compact_custom(i, t+1, inst);
				}
				for(int t = h; t < inst->nnodes-1; t++)
				{
					rmatind[t] = zpos_compact_custom(j, t+1, inst);
				}
				int rmatbeg = 0;			// start positions of the constraint
				sprintf(cname[0], "lazy_const_custom_x(%d,%d)_%d", i+1, j+1, h+1);
				if(CPXaddlazyconstraints(env, lp, rcnt, nzcnt, &rhs, &sense, &rmatbeg, rmatind, rmatval, cname)) 
				{
					print_error(" wrong lazy custom constraint");
				}
			}
		}
	}

	// save model
	if(VERBOSE >= 100)
	{
		CPXwriteprob(env, lp, "tsp_compact_custom.lp", NULL);
	}

	free(cname[0]);
	free(cname);
}


int TSPopt_compact_custom(instance *inst)
{
	
	//inst->tstart = second(); maybe we'll need it  
	//inst->best_lb = -CPX_INFBOUND;   

	// open cplex model
	int i, j, k, l, flag;
	int error, status;
	int cur_numrows, cur_numcols;
	double obj_val;

	CPXENVptr env = CPXopenCPLEX(&error);
	 CPXsetintparam(env, CPXPARAM_Read_DataCheck, 1);			// used to check if there are errors while reading data

	CPXLPptr lp = CPXcreateprob(env, &error, "TSP_COMPACT_CUSTOM");
	CPXsetlogfilename(env, "exec_compact_custom_log.txt", "w");			// it saves the log of the computation in exec_compact_log.txt

	// build model
	build_model_compact_custom(inst, env, lp);

	//printf("\n\nz_pos = %d\n\n", zpos_compact_custom(0,0, inst));
	//printf("\n\nz_pos = %d\n\n", zpos_compact_custom(inst->nnodes-1, inst->nnodes-1, inst));
}


int zpos_compact_custom(int i, int j, instance *inst)
{
	if((i >= inst->nnodes) || (j >= inst->nnodes) || (i<0) || (j<0))
	{
		print_error("Domain contraint not respected zpos_compact_custom");
	}
	int offset = inst->nnodes*(inst->nnodes-1)/2;
	return offset + (i*inst->nnodes + j);
}