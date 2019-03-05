#include "tsp.h"

#define LINE_LENGTH 180

/*   PROTOTYPES   */
void free_instance(instance *inst);
void parse_command_line(int argc, char** argv, instance *inst);
void print_error(const char *err);
void read_input(instance *inst);


void free_instance(instance *inst)
{
	free(inst->xcoord);
	free(inst->ycoord);
}


void parse_command_line(int argc, char** argv, instance *inst) 
{ 
	int i;
	if(VERBOSE >= 100)
		printf(" running %s with %d parameters\n", argv[0], argc-1); 
		
	// default values
	inst->input_file = (char *) calloc(strlen("NULL"), sizeof(char));
	strcpy(inst->input_file, "NULL");
	inst->time_limit = 2147483647;	// random number for now

	int help = 0;
	if(argc < 1) { help = 1; }
	for(i=1; i<argc; i++)
	{
		if(!strcmp(argv[i], "-file") || !strcmp(argv[i],"-f"))
		{
			inst->input_file = (char *) realloc(inst->input_file, strlen(argv[++i])); 
			strcpy(inst->input_file, argv[i]);
		}				// input file
		else if(!strcmp(argv[i],"-time_limit")) { inst->time_limit = atof(argv[++i]); }		// total time limit
		else if(!strcmp(argv[i], "-help") || !strcmp(argv[i], "--help")){ help = 1; break; }		// help mutually exclusive
		else { help = 1; break; }
	}

	if(help || (VERBOSE >= 10))
	{
		printf("\n\n");
		printf("available parameters ---------------------------------------------------\n");
		printf("-file %s\n", inst->input_file); 
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


void read_input(instance *inst)
{
	FILE *fin = fopen(inst->input_file, "r");
    if(fin == NULL) print_error(" input file not found!");
    
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
			if(strncmp(token1, "EUC_2D", 6)) print_error(" format error:  only EDGE_WEIGHT_TYPE == EUC_2D implemented so far!"); 
			active_section = 0;
			continue;
		}

		if(strncmp(par_name, "NODE_COORD_SECTION", 18) == 0)
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
				printf(" ... node %4d at coordinates ( %15.7lf , %15.7lf )\n", i+1, inst->xcoord[i], inst->ycoord[i]); 
			continue;
		}

		printf(" final active section %d\n", active_section);
		print_error(" ... wrong format for the current simplified parser!!!!!!!!!");
	}
	//printf("Passo 1\n");
}