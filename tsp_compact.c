#include "tsp.h"

void build_model_compact(instance *inst, CPXENVptr env, CPXLPptr lp)
{
	int M = inst->nnodes-2;
	double zero = 0.0;
	double one = 1.0;	
	char binary = 'B';
	char general = 'G';

	char **cname = (char **) calloc(1, sizeof(char *));		// (char **) required by cplex...
	cname[0] = (char *) calloc(100, sizeof(char));

	// Adding x_i_j variables
	for(int i=0; i<inst->nnodes; i++)
	{
		for(int j=0; j<inst->nnodes; j++)
		{
			sprintf(cname[0], "x_%d_%d", i+1,j+1);
			double obj = (i==j)?zero : dist(i,j,inst);
			double ub = (i==j)?zero : one;
			if(VERBOSE > 1000)
			{
				printf("The variable %s number %d has value %lf\n", cname[0], xpos_compact(i,j,inst), obj);
			}
			if(CPXnewcols(env, lp, 1, &obj, &zero, &ub, &binary, cname)) 
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
		double lb = one;
		double ub = (i==0)? one : inst->nnodes;
		double obj = zero;
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

	// Adding in degree constraints
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
			if(i == h) 
			{
				if(CPXchgcoef(env, lp, lastrow, xpos_compact(i, h, inst), zero)) 
				{
					print_error(" wrong CPXchgcoef [x1]");
				}
			}
			else
			{
				if(CPXchgcoef(env, lp, lastrow, xpos_compact(i, h, inst), one)) 
				{
					print_error(" wrong CPXchgcoef [x1]");
				}
			}
		}
	}

	// Adding out degree constraints
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
			if(j == h) 
			{
				if(CPXchgcoef(env, lp, lastrow, xpos_compact(h, j, inst), zero)) 
				{
					print_error(" wrong CPXchgcoef [x1]");
				}
			}
			else
			{
				if(CPXchgcoef(env, lp, lastrow, xpos_compact(h, j, inst), one)) 
				{
					print_error(" wrong CPXchgcoef [x1]");
				}
			}
		}
	}

	// Adding lazy constraints (x_i_j + x_j_i <= 1)
	for(int i=0; i<inst->nnodes; i++)
	{
		for(int j=i+1; j<inst->nnodes; j++)
		{
			int lastrow = CPXgetnumrows(env, lp);
			double rhs = 1.0;
			char sense = 'L';
			int rcnt = 1;
			int nzcnt = 2;
			double rmatval = 1.0;
			int rmatind[] = { xpos_compact(i,j, inst), xpos_compact(j,i, inst) };
			int rmatbeg[] = { 0, 2 };
			sprintf(cname[0], "lazy_const(%d, %d)", i+1, j+1);
			if(CPXaddlazyconstraints(env, lp, rcnt, nzcnt, &rhs, &sense, rmatbeg, rmatind, &rmatval, cname)) 
			{
				print_error(" wrong CPXnewrows [x1]");
			}
		}
	}

	// Adding big-M lazy constraints
	for(int i=0; i<inst->nnodes; i++)
	{
		for(int j=1; j<inst->nnodes; j++)
		{
			if(i==j) { continue; }
			int lastrow = CPXgetnumrows(env, lp);
			double rhs = one - M;
			char sense = 'G';
			int rcnt = 1;
			int nzcnt = 3;
		}
	}

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
	const double TOLERANCE = 0.5;
	int i, j, k, l, flag;
	int error, status;
	int cur_numrows, cur_numcols;
	double obj_val;

	CPXENVptr env = CPXopenCPLEX(&error);
	CPXLPptr lp = CPXcreateprob(env, &error, "TSP"); 
	
	// build model
	build_model_compact(inst, env, lp);

	// save model
	if(VERBOSE >= 100)
	{
		CPXwriteprob(env, lp, "tsp.lp", NULL); 
	}
}

int xpos_compact(int i, int j, instance *inst)
{
	if((i >= inst->nnodes) || (j >= inst->nnodes) || (i<0) || (j<0))
	{
		print_error("Domain contraint not respected");
	}
	return (i*inst->nnodes + j);
}