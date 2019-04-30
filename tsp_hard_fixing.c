#include "tsp.h"

void random_fixing(CPXENVptr env, CPXLPptr lp, char *sol_heur, int size, double prob);
void set_default_lb(CPXENVptr env, CPXLPptr lp, char *sol_heur, int size);

int TSP_heur_hf(instance *inst)
{
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

	double values[inst->nnodes]; // values of the variables (1.0)
	int varindices[inst->nnodes]; // indexes of the variables to change
	for(int i = 0; i < inst->nnodes; i++)
	{
		values[i] = 1.0;
	}

	for(int i = 0; i < inst->nnodes; i++)
	{
		varindices[i] = xpos(i, (i+1)%inst->nnodes, inst);
		if(VERBOSE > 1000)
		{
			printf("arco inserito: %d %d\n", i, (i+1)%inst->nnodes);
		}
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

	/*
	unsigned long start = microseconds();
	while(!done)
	{
		unsigned long elapsed = microseconds();
		if((double)(elapsed-start)/1000000 >= inst->time_limit)
		{
			done = 1;
		}
		else
		{

		}
	}*/

	return 0;

}


void set_default_lb(CPXENVptr env, CPXLPptr lp, char *sol_heur, int size)
{
	double zero = 0.0;
	char lb = 'L';

	for(int i = 0; i < size; i++)
	{
		CPXchgbds(env, lp, 1, &i, &lb, &zero);
	}
}

void random_fixing(CPXENVptr env, CPXLPptr lp, char *sol_heur, int size, double prob)
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
		rand = random() / RAND_MAX;
		
		if(sol_heur[i] > 0.5 && rand < prob)
		{
			CPXchgbds(env, lp, 1, &i, &lb, &one);
		}
	}
}


