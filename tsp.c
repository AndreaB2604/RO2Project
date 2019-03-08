#include "tsp.h"

/* PROTOTYPES */
void build_model(instance *inst, CPXENVptr env, CPXLPptr lp);
double dist(int i, int j, instance *inst);
int xpos(int i, int j, instance *inst);
void print_error(const char *err);
double sqrt(double x);
int TSPopt(instance *inst);    

double dist(int i, int j, instance *inst)
{
	double dx = inst->xcoord[i] - inst->xcoord[j];
	double dy = inst->ycoord[i] - inst->ycoord[j];
	int dis = sqrt(dx*dx + dy*dy) + 0.5; // nearest integer 
	return ((double) dis);
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

int TSPopt(instance *inst)
{
	//inst->tstart = second(); maybe we'll need it  
	//inst->best_lb = -CPX_INFBOUND;   

	// open cplex model
	int error;
	CPXENVptr env = CPXopenCPLEX(&error);
	CPXLPptr lp = CPXcreateprob(env, &error, "TSP"); 
	
	//build model
	build_model(inst, env, lp);

	//save model
	if(VERBOSE >= 100)
	{
		CPXwriteprob(env, lp, "tsp.lp", NULL); 
	}

	return 0;
}

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
			if(VERBOSE > 1000)
			{
				printf("Variabile %s numero %d contribuisce con %lf\n", cname[0], xpos(i,j,inst), obj);
			}
			double ub = 1.0; // recall we deal with {0,1} variables
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
			if(i==h) {continue;}
			if(CPXchgcoef(env, lp, lastrow, xpos(h, i, inst), 1.0)) 
			{
				print_error(" wrong CPXchgcoef [x1]");
			}
		}
	}
	free(cname[0]);
	free(cname);
}
