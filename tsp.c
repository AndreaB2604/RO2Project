#include "tsp.h"

/* PROTOTYPES */
void build_model(instance *inst);//, CPXENVptr env, CPXLPptr lp);
double dist(int i, int j, instance *inst);
int xpos(int i, int j, instance *inst);
void print_error(const char *err);
double sqrt(double x);

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
		print_error("Error: i==j");
	if((i >= inst->nnodes) || (j >= inst->nnodes) || (i<0) || (j<0))
		print_error("Domain contraint not respected");
	if(i > j)
		return xpos(j, i, inst);
	return (i * inst->nnodes + j - (i+1)*(i+2)/2);
}

void build_model(instance *inst) //, CPXENVptr env, CPXLPptr lp)
{
	double zero = 0.0; // one = 1.0; 	
	char binary = 'B';

	char **cname = (char **) calloc(1, sizeof(char *));		// (char **) required by cplex...
	cname[0] = (char *) calloc(100, sizeof(char));

	for(int i = 0; i < inst->nnodes; i++)
	{
		for(int j = i+1; j < inst->nnodes; j++ )
		{
			sprintf(cname[0], "x_%d_%d", i+1,j+1);
			printf("Variabile %s numero %d", cname[0], xpos(i,j,inst));
			double obj = dist(i,j,inst);   
			double ub = (i == j)? 0.0 : 1.0;
			//if(CPXnewcols(env, lp, 1, &obj, &zero, &ub, &binary, cname)) 
			//	print_error(" wrong CPXnewcols on x var.s");
    		//if(CPXgetnumcols(env,lp)-1 != xpos(i,j, inst))
    		//	print_error(" wrong position for x var.s");
		}
	}
}
