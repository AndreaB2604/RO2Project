#include "tsp.h"

void build_model(instance *inst, CPXENVptr env, CPXLPptr lp)
{
	double zero = 0.0; // one = 1.0; 	
	char binary = 'B';

	char **cname = (char **) calloc(1, sizeof(char *));		// (char **) required by cplex...
	cname[0] = (char *) calloc(100, sizeof(char));

	for(int i = 0; i < inst->nnodes; i++)
	{
		for(int j = i+1; j < inst->nnodes; j++ )
		{
			// Adding variables to model + their respective cost in the
			// objective function
			sprintf(cname[0], "x_%d_%d", i+1,j+1);
			double obj = dist(i,j,inst);   
			double ub = 1.0; // recall we deal with {0,1} variables

			if(VERBOSE > 1000)
			{
				printf("The variable %s number %d has value %lf\n", cname[0], xpos(i,j,inst), obj);
			}
			if(CPXnewcols(env, lp, 1, &obj, &zero, &ub, &binary, cname)) 
			{
				print_error(" wrong CPXnewcols on x var.s");
			}
			if(CPXgetnumcols(env,lp)-1 != xpos(i,j, inst))
			{
				print_error(" wrong position for x var.s");
			}
		}
	}
	
	for(int h = 0; h < inst->nnodes; h++)  
	{
		// Adding the constraint that each node has to have 2 adjacent edges selected
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
			if(i==h) { continue; }
			if(CPXchgcoef(env, lp, lastrow, xpos(h, i, inst), 1.0)) 
			{
				print_error(" wrong CPXchgcoef [x1]");
			}
		}
	}
	free(cname[0]);
	free(cname);
}


double dist(int i, int j, instance *inst)
{
	double dx = inst->xcoord[i] - inst->xcoord[j];
	double dy = inst->ycoord[i] - inst->ycoord[j];
	int dis = sqrt(dx*dx + dy*dy) + 0.5; // nearest integer 
	return ((double) dis);
}


int TSPopt(instance *inst)
{
	//inst->tstart = second(); maybe we'll need it  
	//inst->best_lb = -CPX_INFBOUND;   

	// open cplex model
	const double TOLERANCE = 0.5;
	int i, j, k, l, flag;
	int error, status;
	int cur_numrows, cur_numcols;
	double obj_val;

	CPXENVptr env = CPXopenCPLEX(&error);
	CPXLPptr lp = CPXcreateprob(env, &error, "TSP"); 
	
	// build model
	build_model(inst, env, lp);

	// save model
	if(VERBOSE >= 100)
	{
		CPXwriteprob(env, lp, "tsp.lp", NULL); 
	}

	// solve the optimisation problem
	if(CPXmipopt(env, lp))
	{
		print_error("Optimisation failed in TSPopt()");
	}

	// get the best solution and print it
	if(CPXgetobjval(env, lp, &obj_val))
	{
		print_error("Optimisation failed in TSPopt()");
	}
	printf("Solution value  = %lf\n\n", obj_val);

	// cur_numrows is the number of nodes == inst->nnodes
	// cur_numcols is the number of variables 
	cur_numrows = CPXgetnumrows(env, lp);
	cur_numcols = CPXgetnumcols(env, lp);
	
	// get the optimal solution of the variables
	inst->best_sol = (double *) calloc(cur_numcols, sizeof(double));
	if(CPXgetx(env, lp, inst->best_sol, 0, cur_numcols-1))
	{
		print_error("Optimisation failed in TSPopt()");
	}

	// print only the non-zero variables
	if(VERBOSE > 1000)
	{
		for(k = 0; k < cur_numcols; k++)
		{	
			l = inst->nnodes -1;
			flag = 0;
			for(i=0; (i<inst->nnodes-1) && (!flag); i++)
			{
				if(k<l)
				{
					for(j=i+1; j<inst->nnodes; j++)
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
	//printf("cur_numcols = %d\n", xpos(inst->nnodes-2, inst->nnodes-1, inst));
	//printf("cur_numcols = %d\n", cur_numcols);

	/* Free up the problem as allocated by CPXcreateprob, if necessary */
	if(lp != NULL)
	{
		status = CPXfreeprob(env, &lp);
	}

	/* Free up the CPLEX environment, if necessary */
	if(env != NULL) 
	{
		status = CPXcloseCPLEX(&env);
	}

	return 0;
}


int xpos(int i, int j, instance *inst)
{
	if(i==j)
	{
		print_error("Error: i==j");
	}
	if((i >= inst->nnodes) || (j >= inst->nnodes) || (i<0) || (j<0))
	{
		print_error("Domain contraint not respected");
	}
	if(i > j)
	{
		return xpos(j, i, inst);
	}
	return (i * inst->nnodes + j - (i+1)*(i+2)/2);
}
