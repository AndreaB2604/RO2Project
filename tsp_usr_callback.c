#include "tsp.h"

static int CPXPUBLIC cutcallback(CPXCENVptr env, void *cbdata, int wherefrom, void *cbhandle, int *useraction_p);
//int myfunction(int ncount, int ecount, int *elist, double *x, int *ncomp, int **compscount, int **comps);
int doit_fn_concorde(double cutval, int cutcount, int *cut, void *inParam);
int separationMultiple(instance *inst, int ncomp, int *compscount, int *comps, CPXCENVptr env, void *cbdata, int wherefrom);
static int CPXPUBLIC mylazycallback(CPXCENVptr env, void *cbdata, int wherefrom, void *cbhandle, int *useraction_p);

const double EPS = 0.1;
int TSPopt_usr_callback(instance *inst)
{
	// open cplex model
	int error, status;
	int done = 0;
	int cur_numrows, cur_numcols;
	double obj_val;

	CPXENVptr env = CPXopenCPLEX(&error);
	CPXsetintparam(env, CPXPARAM_Read_DataCheck, 1);			// used to check if there are errors while reading data

	CPXLPptr lp = CPXcreateprob(env, &error, "TSP"); 
	if(VERBOSE > 50)
	{
		CPXsetlogfilename(env, "exec_log.txt", "w");			// it saves the log of the computation in exec_compact_log.txt
	}

	printf("User callback model\n");
	// build model
	build_model(inst, env, lp);

	CPXsetintparam(env, CPX_PARAM_MIPCBREDLP, CPX_OFF); // let MIP callbacks work on the original model
	//CPXsetintparam(env, CPX_PARAM_PRELINEAR, 0);
	//CPXsetintparam(env, CPX_PARAM_REDUCE, CPX_PREREDUCE_PRIMALONLY);
	CPXsetusercutcallbackfunc(env, cutcallback, inst);
	CPXsetlazyconstraintcallbackfunc(env, mylazycallback, inst);
	int ncores = 1;
	CPXgetnumcores(env, &ncores);
	CPXsetintparam(env, CPX_PARAM_THREADS, ncores); // reset after callback
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
	if(VERBOSE >= 10)
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
	printf("Execution dettime of usrcut_callback = %.3f ticks\n", exec_det_time);
	printf("Execution time of usrcut_callback = %.3f s\n", ((double)exec_time/1000000));

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

static int CPXPUBLIC cutcallback(CPXCENVptr env, void *cbdata, int wherefrom, void *cbhandle, int *useraction_p)
{
	*useraction_p = CPX_CALLBACK_DEFAULT; 
	instance* inst = (instance *) cbhandle; 			// casting of cbhandle    

	int status;
	int active = 0; // active = 0 then use the depth limit, active = 1 always separate solution
	int mynode = 0;
	CPXINT depth = 0;
	CPXINT seqn = 0;
	if(VERBOSE > 1000)
	{
		if(CPXgetcallbackinfo(env, cbdata, wherefrom, CPX_CALLBACK_INFO_NODES_LEFT, &mynode))
		{
			print_error("Error in getting node number");
		}
		mynode -= 1;
		if(status = CPXgetcallbacknodeinfo(env, cbdata, wherefrom, 0, CPX_CALLBACK_INFO_NODE_SEQNUM, &seqn))
		{
			printf("Node is: %d\n", (mynode+1));
			printf("Status is %d\n", status);
			print_error("Error in getting the node depth");
		}
	}
	// Getting the depth of the current node
	if(status = CPXgetcallbacknodeinfo(env, cbdata, wherefrom, 0, CPX_CALLBACK_INFO_NODE_DEPTH, &depth))
	{
		printf("Node is: %d\n", (mynode+1));
		printf("Status is %d\n", status);
		print_error("Error in getting the node depth");
	}
	if(VERBOSE > 1000)
	{
		printf("Node sequence: %d\n", (seqn));
		printf("Node is: %d\n", (mynode+1));
		printf("Node depth is: %d\n", depth);
	}
	// Applying the separation only if the depth is <= 10
	if(depth > 10 && !active)
	{
		return 0;
	}

	int edgecount = inst->nnodes * (inst->nnodes - 1) / 2;
	int idx_elist = 0; // index for filling edge list
	int elist [edgecount * 2]; // edge in node node format
	for(int i=0; i< inst->nnodes; i++)
	{
		for(int j=i+1; j < inst->nnodes; j++)
		{
			elist[idx_elist++] = i;
			elist[idx_elist++] = j;
		}
	}
	double *xstar = (double*) malloc(edgecount * sizeof(double)); // curr solution
	if ( CPXgetcallbacknodex(env, cbdata, wherefrom, xstar, 0, edgecount-1) )
	{
		printf("Error in retrieving solution usercutcallback\n");
		return 1; // y = current y from CPLEX-- y starts from position 0
	}
	int ncomp = 0;
	int *comps = (int*) malloc(inst->nnodes * sizeof(int));
	int *compscount = (int*) malloc(inst->nnodes * sizeof(int));

	if(CCcut_connect_components(inst->nnodes, edgecount, elist, xstar, &ncomp, &compscount, &comps))
	//if(myfunction(inst->nnodes, edgecount, elist, xstar, &ncomp, &compscount, & comps))
	{
		printf("Error in getting components of solution usercutcallback\n");
		return 1; // y = current y from CPLEX-- y starts from position 0
	}
	
	if(ncomp > 1)
	{
		// Add constraints on connected components
		int ncuts = separationMultiple(inst, ncomp, compscount, comps, env, cbdata, wherefrom);	    
		*useraction_p = CPX_CALLBACK_SET; 		// tell CPLEX that cuts have been created
		if(VERBOSE > 100)
		{
			printf("Cuts added %d\n", ncuts);
		}
	}

	if(ncomp == 1)
	{
		cc_instance instCC = {.env = env, .cbdata = cbdata, .wherefrom = wherefrom, .useraction_p = useraction_p, .inst = inst };	
		// Separate fractionary solution
		if(CCcut_violated_cuts(inst->nnodes, edgecount, elist, xstar, 2.0 - EPS, doit_fn_concorde, (void*) &instCC))
		{
			printf("Error in computing flow usercutcallback\n");
			return 1; // y = current y from CPLEX-- y starts from position 0
		}
		*useraction_p = CPX_CALLBACK_SET; 		// tell CPLEX that cuts have been created
	}
	
	if(ncomp < 1)
	{
		print_error("Error ncomp must be >= 1 usercutcallback\n");
		return 1; // y = current y from CPLEX-- y starts from position 0

	}
	if(VERBOSE > 1000)
	{
		printf("Num components is : <%d> \n ", ncomp);
	}

	// free the space allocated with malloc
	free(xstar);
	free(comps);
	free(compscount);
	return 0;
}
int doit_fn_concorde(double cutval, int cutcount, int *cut, void *inParam)
{
	int i,j;
	cc_instance* instCC = (cc_instance *) inParam; 			// casting of inParam 
	if(VERBOSE > 1000)
	{
		printf("Component ---------------------------------\n");
		for(int i=0; i < cutcount; i++)
		{
			printf("Node %d is in this cut\n", (cut[i]+1));
		}
		printf("-------------------------------------------\n");
	}
	
	int num_x_var = (instCC->inst->nnodes-1) * instCC->inst->nnodes / 2; 	// number of x variables
	if(VERBOSE > 100)
	{
		printf("number of variables %d\n", num_x_var);
	}
	int nzcnt = 0;					// number of non-zero variables in the constraint
	char sense = 'L';
	double rmatval[num_x_var];		// coefficients of the non-zero variables
	int rmatind[num_x_var]; 		// position of the variables to set (in terms of columns)
	int rhs = cutcount -1;
	for(int i = 0; i < num_x_var; i++)
	{
		rmatval[i] = 0;
	}
	
	for(i=0; i < cutcount; ++i)
	{
		for(j=0; j < cutcount; ++j)
		{
			if(cut[i] >= cut[j])
			{
				continue;
			}
			else
			{
				if(VERBOSE > 1000)
				{
					printf("edge x_%d_%d in pos %d\n", (cut[i]+1),(cut[j]+1),xpos(cut[i], cut[j], instCC->inst));
				}
				rmatind[nzcnt] = xpos(cut[i], cut[j], instCC->inst);
				rmatval[nzcnt] = 1.0;
				nzcnt++;
			}
		}
	}
	
	if(CPXcutcallbackadd(instCC->env, instCC->cbdata, instCC->wherefrom, nzcnt, rhs, sense, rmatind, rmatval, CPX_USECUT_FORCE))
	{
			print_error("USER_separation: CPXcutcallbackadd error single component");
	}
	if(VERBOSE > 100)
	{
		printf("Cut added with one connected component\n");
	}
	return 0;
}
int separationMultiple(instance *inst, int ncomp, int *compscount, int *comps, CPXCENVptr env, void *cbdata, int wherefrom)
{
	
	int num_x_var = (inst->nnodes-1) * inst->nnodes / 2; 	// number of x variables
	int offset = 0;
	int added = 0;
	for(int i = 0; i < ncomp; i++)
	{
		int nzcnt = 0;					// number of non-zero variables in the constraint
		char sense = 'L';
		double rmatval[num_x_var];		// coefficients of the non-zero variables
		int rmatind[num_x_var]; 		// position of the variables to set (in terms of columns)

		for(int i = 0; i < num_x_var; i++)
		{
			rmatval[i] = 0;
		}
		
		int comp_vertexes = compscount[i];
		double rhs = comp_vertexes-1.0;

		if(VERBOSE > 1000)
		{
			printf("-------------------------------------------------\n");
			printf("%d component has %d nodes\n", i , comp_vertexes);
		}
		int j,k;
		for(j=offset; j < offset+comp_vertexes; j++)
		{
			for(k=offset; k < offset+comp_vertexes; k++)
			{
				if((comps[j]==comps[k]) || (comps[j]>comps[k]))
				{ 
					continue; 
				}
				else
				{
					if(VERBOSE > 1000)
					{
						printf("variable x_%d_%d in %d\n", (comps[j]+1), (comps[k]+1), xpos(comps[j], comps[k], inst));
					}
					rmatind[nzcnt] = xpos(comps[j], comps[k], inst);
					rmatval[nzcnt] = 1.0;
					nzcnt++;
				}
			}
			if(VERBOSE > 1000)
			{
				printf("node %d\n", (int) (comps[j]));
			}
		}
		if(VERBOSE > 1000)
		{
			printf("-------------------------------------------------\n");
		}
		offset +=comp_vertexes;
		if(CPXcutcallbackadd(env, cbdata, wherefrom, nzcnt, rhs, sense, rmatind, rmatval, CPX_USECUT_FORCE))
		{
			print_error("USER_separation: CPXcutcallbackadd error user callback");
		}
		added++;
	}
	if(VERBOSE > 100)
	{
		printf("number of cuts added in the subfunction is %d\n", added);
	}
	return ncomp;
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
