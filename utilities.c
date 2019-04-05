#include "tsp.h"

#define LINE_LENGTH 180

double dist_att(int i, int j, instance *inst);
double dist_euc2D(int i, int j, instance *inst);
double dist_geo(int i, int j, instance *inst);
void print_plot_subtour(instance *inst, char *plot_file_name);
void print_plot_mtz(instance *inst, char *plot_file_name);
void print_plot_compact_custom(instance *inst, char *plot_file_name);

// this function computes the connected components where:
// columns_number is the number of variables of the model
// node_components will be an array that will contain in the i-th position the number of the connected component of the i-th node
// components_value will be an array of length number_cc that will contain only the values of the different connected components
// number_cc is a pointer that will contain the number of connected components of the graph
// NOTE: the instance is needed only for the xpos function and the number of nodes
void connected_components(instance *inst, double *best_sol, int columns_number, int *nodes_components, int *components_values, int *number_cc)
{
	for(int i = 0; i < inst->nnodes; i++)
	{
		nodes_components[i] = i;		// components go from 0 to n-1
	}

	for(int k = 0; k < columns_number; k++)
	{
		if(best_sol[k] > TOLERANCE)
		{
			int l = inst->nnodes-1;
			int flag = 0;
			for(int i = 0; (i < inst->nnodes-1) && (!flag); i++)
			{
				if(k < l)
				{
					for(int j = i+1; j < inst->nnodes; j++)
					{
						// if the {i,j} exists then i and j belong to the same connected component
						if((xpos(i, j, inst) == k)) 
						{
							if(nodes_components[i] != nodes_components[j])
							{
								int ci = nodes_components[i];
								int cj = nodes_components[j];
								for(int n = 0; n < inst->nnodes; n++)
								{
									if(nodes_components[n] == cj)
									{
										nodes_components[n] = ci;
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
	}

	for(int i = 0; i < inst->nnodes; i++)
	{
		components_values[i] = -1;
	}

	int num_comp = 0; // number of current connected components
	// compute what the connected components are and their number
	for(int i = 0; i < inst->nnodes; i++)
	{
		for(int j = 0; (j < inst->nnodes) && (nodes_components[i] != components_values[j]); j++)
		{
			if(components_values[j] == -1)
			{
				components_values[j] = nodes_components[i];
				num_comp++;
				break;
			}
		}
	}
	
	*number_cc = num_comp;
}

double dist(int i, int j, instance *inst)
{
	if(!strncmp(inst->dist_type, "EUC_2D", 6))
		return dist_euc2D(i, j, inst);
	else if(!strncmp(inst->dist_type, "GEO", 3))
		return dist_geo(i, j, inst);
	else if(!strncmp(inst->dist_type, "ATT", 3))
		return dist_att(i, j, inst);
	else
		print_error(" format error:  only ATT, EUC_2D and GEO distances implemented so far!");
}

double dist_att(int i, int j, instance *inst)
{
	double dx = inst->xcoord[i] - inst->xcoord[j];
	double dy = inst->ycoord[i] - inst->ycoord[j];

	double rij = sqrt((dx*dx + dy*dy) / 10.0);
	double tij = (int) (rij + 0.5);

	if(tij < rij)
		return tij + 1;
	else
		return tij;
}

double dist_euc2D(int i, int j, instance *inst)
{
	double dx = inst->xcoord[i] - inst->xcoord[j];
	double dy = inst->ycoord[i] - inst->ycoord[j];
	int dis = sqrt(dx*dx + dy*dy) + 0.5; // nearest integer 
	return ((double) dis);
}

double dist_geo(int i, int j, instance *inst)
{
	double const PI = 3.141592;
	double const RRR = 6378.388;
	// compute latutude and longitude of inst->nnodes[i]
	double deg = (int) inst->xcoord[i];
	double min = inst->xcoord[i] - deg;
	double latitude_i = PI * (deg + 5.0 * min / 3.0 ) / 180.0;
	deg = (int) inst->ycoord[i];
	min = inst->ycoord[i] - deg;
	double longitude_i = PI * (deg + 5.0 * min / 3.0 ) / 180.0;

	// compute latutude and longitude of inst->nnodes[j]
	deg = (int) inst->xcoord[j];
	min = inst->xcoord[j] - deg;
	double latitude_j = PI * (deg + 5.0 * min / 3.0 ) / 180.0;
	deg = (int) inst->ycoord[j];
	min = inst->ycoord[j] - deg;
	double longitude_j = PI * (deg + 5.0 * min / 3.0 ) / 180.0;

	double q1 = cos(longitude_i - longitude_j);
	double q2 = cos(latitude_i - latitude_j);
	double q3 = cos(latitude_i + latitude_j);
	double dij = (int) (RRR * acos( 0.5*((1.0+q1)*q2 - (1.0-q1)*q3) ) + 1.0);
	return dij;
}

void free_instance(instance *inst)
{
	if(inst->xcoord != NULL) free(inst->xcoord);
	if(inst->ycoord != NULL) free(inst->ycoord);
	if(inst->input_file != NULL) free(inst->input_file);
	if(inst->dist_type != NULL) free(inst->dist_type);
	if(inst->best_sol != NULL) free(inst->best_sol);
}

void parse_command_line(int argc, char** argv, instance *inst) 
{ 
	int i;
	if(VERBOSE >= 100)
		printf(" running %s with %d parameters\n", argv[0], argc-1); 
		
	// default values
	inst->input_file = (char *) calloc(strlen("NULL"), sizeof(char));
	strcpy(inst->input_file, "NULL");

	inst->model_type = (char *) calloc(strlen("NULL"), sizeof(char));
	strcpy(inst->model_type, "NULL");

	inst->time_limit = 2147483647;	// random number for now

	int help = 0;
	if(argc < 1) { help = 1; }
	for(i=1; i<argc; i++)
	{
		// input file
		int param_exists = (argc != i+1);
		if(!strcmp(argv[i], "-file") || !strcmp(argv[i],"-f") && param_exists)
		{
			inst->input_file = (char *) realloc(inst->input_file, strlen(argv[++i])); 
			strcpy(inst->input_file, argv[i]);
		}
		else if(!strcmp(argv[i],"-time_limit") && param_exists)		// total time limit
		{
			inst->time_limit = atof(argv[++i]);
		}
		else if(!strcmp(argv[i],"-model") && param_exists)	// model type
		{
			inst->model_type = (char *) realloc(inst->model_type, strlen(argv[++i]));
			strcpy(inst->model_type, argv[i]);
			if(strncmp(inst->model_type, "subtour", 7) && strncmp(inst->model_type, "mtz", 3) && strncmp(inst->model_type, "compact_custom", 14) && strncmp(inst->model_type, "sec_loop", 8) && strncmp(inst->model_type, "sec_callback", 12))  
		    {
		    	printf("\n\nModel type non supported, choose between:\n");
		    	printf("subtour : optimisation without SECs\n");
		    	printf("sec_loop: optimisation with SECs using the loop model\n");
		    	printf("mtz : optimisation using the mtz constraints\n");
		    	printf("compact_custom : optimisation using the compact custom model\n");
		    	fflush(NULL); 
				exit(1);
		    }
		} 	
		else if(!strcmp(argv[i], "-help") || !strcmp(argv[i], "--help")) { help = 1; break; }		// help mutually exclusive
		else { help = 1; break; }
	}

	if(help || (VERBOSE >= 10))
	{
		printf("\n\n");
		printf("available parameters ---------------------------------------------------\n");
		printf("-file %s\n", inst->input_file); 
		printf("-model %s\n", inst->model_type);
		printf("-time_limit %lf\n", inst->time_limit);
		printf("\nenter -help or --help for help\n");
		printf("------------------------------------------------------------------------\n\n");
	}

	if(help) exit(1);
}


void print_error(const char *err)
{
	printf("\n\n ERROR: %s \n\n", err); 
	fflush(NULL); 
	exit(1);
}

void print_plot(instance *inst, char *plot_file_name)
{
	if(!strncmp(inst->model_type, "subtour", 7) || !strncmp(inst->model_type, "sec_loop", 8) || !strncmp(inst->model_type, "sec_callback", 12))
		return print_plot_subtour(inst, plot_file_name);
	else if(!strncmp(inst->model_type, "mtz", 3))
		return print_plot_mtz(inst, plot_file_name);
	else if(!strncmp(inst->model_type, "compact_custom", 14))
		return print_plot_compact_custom(inst, plot_file_name);
	else 
		print_error(" format error: plot non supported in print_plot()!");
}

void print_plot_subtour(instance *inst, char *plot_file_name)
{
	int i, j, k, l, flag;
	int cur_numcols = xpos(inst->nnodes-2, inst->nnodes-1, inst)+1; // this is equal to n*(n-1)/2
	FILE *file = fopen(plot_file_name, "w");
	fprintf(file, "%d\n", inst->nnodes);
	for(i=0; i<inst->nnodes; i++)
	{
		fprintf(file, "%lf %lf\n", inst->xcoord[i], inst->ycoord[i]);
	}
	
	fprintf(file, "\nNON ZERO VARIABLES\n");

	for(k=0; k<cur_numcols; k++)
	{
		if(inst->best_sol[k] > TOLERANCE)
		{
			l = inst->nnodes -1;
			flag = 0;
			for(i=0; (i<inst->nnodes-1) && (!(flag)); i++)
			{
				if(k<l)
				{
					for(j=i+1; j<inst->nnodes; j++)
					{
						if(xpos(i, j, inst) == k) 
						{
							fprintf(file, "x_%d_%d = %f\n", i+1, j+1, inst->best_sol[k]);
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
	fclose(file);
}

void print_plot_mtz(instance *inst, char *plot_file_name)
{

	int i,j,k;
	FILE *file = fopen(plot_file_name, "w");
	int max_idx_x = inst->nnodes * inst->nnodes;
	fprintf(file, "%d\n", inst->nnodes);
	for(i=0; i<inst->nnodes; i++)
	{
		fprintf(file, "%lf %lf\n", inst->xcoord[i], inst->ycoord[i]);
	}

	fprintf(file, "\nNON ZERO VARIABLES\n");
	// print the x variables that are non-zero
	for(k = 0; k < max_idx_x; k++)
	{
		if(inst->best_sol[k] > TOLERANCE)
		{
			for(i=0; i<inst->nnodes; i++)
			{
				for(j=0; j<inst->nnodes; j++)
				{
					if((i!=j) && (xpos_mtz(i, j, inst) == k)) 
					{
						fprintf(file, "x_%d_%d = %f\n", i+1, j+1, inst->best_sol[k]);
					}
				}
			}	
		}
	}
	
	fclose(file);
}

void print_plot_compact_custom(instance *inst, char *plot_file_name)
{

	int i,j,k;
	FILE *file = fopen(plot_file_name, "w");
	int max_idx_x = inst->nnodes * (inst->nnodes-1) / 2; 	// == xpos(inst->nnodes-1, inst->nnodes-1, inst)
	fprintf(file, "%d\n", inst->nnodes);
	for(i=0; i<inst->nnodes; i++)
	{
		fprintf(file, "%lf %lf\n", inst->xcoord[i], inst->ycoord[i]);
	}

	fprintf(file, "\nNON ZERO VARIABLES\n");
	// print the x variables that are non-zero
	for(int k = 0; k < max_idx_x; k++)
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
							fprintf(file, "x_%d_%d = %f\n", i+1, j+1, inst->best_sol[k]);
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
	fclose(file);
}

void read_input(instance *inst)
{
	FILE *fin = fopen(inst->input_file, "r");
    if(fin == NULL) print_error("input file not found!");

    if(!strncmp(inst->model_type, "NULL", 4))
    	print_error("model type not specified!");
    inst->nnodes = -1;

    char line[LINE_LENGTH];
    
    char *par_name;   
    char *token1;
    char *token2;
    
    int active_section = 0; // =1 NODE_COORD_SECTION, =2 DEMAND_SECTION, =3 DEPOT_SECTION 
    
    int do_print = (VERBOSE >= 1000);
	
    while(fgets(line, sizeof(line), fin) != NULL) 
	{
		if(VERBOSE >= 2000) { printf("%s",line); fflush(NULL); }
		if(strlen(line) <= 1) continue; // skip empty lines
		par_name = strtok(line, " :");
		if(VERBOSE >= 3000) { printf("parameter \"%s\"", par_name); fflush(NULL); }

		if(!strncmp(par_name, "NAME", 4)) 
		{
			active_section = 0;
			continue;
		}

		if(!strncmp(par_name, "COMMENT", 7)) 
		{
			active_section = 0;   
			token1 = strtok(NULL, "");  
			// if(VERBOSE >= 10) printf(" ... solving instance %s with model %d\n\n", token1, inst->model_type);
			continue;
		}

		if(!strncmp(par_name, "TYPE", 4)) 
		{
			token1 = strtok(NULL, " :");  
			if(strncmp(token1, "TSP", 3)) print_error(" format error:  only TYPE == TSP implemented so far!"); 
			active_section = 0;
			continue;
		}

		if(!strncmp(par_name, "DIMENSION", 9)) 
		{
			if(inst->nnodes >= 0) print_error(" repeated DIMENSION section in input file");
			token1 = strtok(NULL, " :");
			inst->nnodes = atoi(token1);
			if(do_print) printf(" ... nnodes %d\n", inst->nnodes); 
			// inst->demand = (double *) calloc(inst->nnodes, sizeof(double)); 	 
			inst->xcoord = (double *) calloc(inst->nnodes, sizeof(double)); 	 
			inst->ycoord = (double *) calloc(inst->nnodes, sizeof(double));    
			active_section = 0;  
			continue;
		}


		// ATTENZIONE NEI FILE .tsp CI SONO ANCHE DISTANZE DI TIPO "ATT" E "EXPLICIT"
		// E A VOLTE VIENE PASSATA LA MATRICE DELLE ISTANZE SPECIFICATA NEL CAMPO "EDGE_WEIGHT_FORMAT"
		// DA CHIEDERE...
		if(!strncmp(par_name, "EDGE_WEIGHT_TYPE", 16)) 
		{
			token1 = strtok(NULL, " :");
			if(!strncmp(token1, "EUC_2D", 6))
			{
				inst->dist_type = (char *) calloc(strlen(token1), sizeof(char));
				strcpy(inst->dist_type, token1);
			}
			else if(!strncmp(token1, "GEO", 3))
			{
				inst->dist_type = (char *) calloc(strlen(token1), sizeof(char));
				strcpy(inst->dist_type, token1);
			}
			else if(!strncmp(token1, "ATT", 3))
			{
				inst->dist_type = (char *) calloc(strlen(token1), sizeof(char));
				strcpy(inst->dist_type, token1);
			}
			else
			{
				print_error(" format error:  only ATT, EUC_2D and GEO distances implemented so far!");
			}
			active_section = 0;
			continue;
		}

		if(!strncmp(par_name, "DISPLAY_DATA_TYPE", 17))
		{
			token1 = strtok(NULL, " :");
			if(strncmp(token1, "COORD_DISPLAY", 13) && strncmp(token1, "TWOD_DISPLAY", 12))
				print_error(" format error: DYSPLAY_DATA_TYPE parameter not supported!");
			active_section = 0;
			continue;
		}

		if(!strncmp(par_name, "NODE_COORD_SECTION", 18))
		{
			if(inst->nnodes <= 0) 
				print_error(" ... DIMENSION section should appear before NODE_COORD_SECTION section");
			active_section = 1;
			continue;
		}

		// DA CHIEDERE ANCHE GLI ALTRI if SE POSSONO SERVIRE E PERCHÉ

		if(!strncmp(par_name, "EOF", 3)) 
		{
			active_section = 0;
			break;
		}

		if(active_section == 1) // within NODE_COORD_SECTION
		{
			int i = atoi(par_name) - 1; 
			if(i < 0 || i >= inst->nnodes)
				print_error(" ... unknown node in NODE_COORD_SECTION section");     
			token1 = strtok(NULL, " :,");
			token2 = strtok(NULL, " :,");
			inst->xcoord[i] = atof(token1);
			inst->ycoord[i] = atof(token2);
			if(do_print)
				printf(" ... node %d at coordinates ( %.f , %f )\n", i+1, inst->xcoord[i], inst->ycoord[i]); 
			continue;
		}

		printf(" final active section %d\n", active_section);
		print_error(" ... wrong format for the current simplified parser!!!!!!!!!");
	}
}
