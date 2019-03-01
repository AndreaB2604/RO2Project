#include "tsp.h"

#define LINE_LENGTH 180
/*   PROTOTYPES   */
void parse_command_line(int argc, char** argv, instance *inst);
void read_input(instance *inst);

/*   MAIN   */
int main(int argc, char **argv)
{
	instance inst;
	if(argc < 2)
	{
		printf("Usage: %s -help for help\n", argv[0]);
		exit(1);
	}

	parse_command_line(argc, argv, &inst);

	read_input(&inst);
	return 0;
}


void parse_command_line(int argc, char** argv, instance *inst) 
{ 
	int i;
	if(VERBOSE >= 100)
		printf(" running %s with %d parameters\n", argv[0], argc-1); 
		
	// default values
	strcpy(inst->input_file, "NULL");
	inst->time_limit = 2147483647;	// random number for now

	int help = 0;
	if(argc < 1) { help = 1; }
	for(i=1; i<argc; i++)
	{
		if(!strcmp(argv[i], "-file")) { strcpy(inst->input_file,argv[++i]); }				// input file
		else if(!strcmp(argv[i],"-f")) { strcpy(inst->input_file,argv[++i]); }				// input file
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

void read_input(instance *inst)
{
	FILE *fin = fopen(inst->input_file, "r");
    if ( fin == NULL ) print_error(" input file not found!");
    
    inst->nnodes = -1;

    char line[LINE_LENGTH];
    char *par_name;   
    char *token1;
    char *token2;
    
    int active_section = 0; // =1 NODE_COORD_SECTION, =2 DEMAND_SECTION, =3 DEPOT_SECTION 
    
    int do_print = ( VERBOSE >= 1000 );
	
}
