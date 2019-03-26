#include "tsp.h"

/*   MAIN   */
int main(int argc, char **argv)
{
	instance inst;
	int model_chosen;

	if(argc < 2)
	{
		printf("Usage : %s -help for help\n", argv[0]);
		exit(1);
	}

	parse_command_line(argc, argv, &inst);

	read_input(&inst);
	
	// If we will want to save the file to plot in the same directory of the file read
	// char *file_to_plot = (char *) malloc(strlen(inst.input_file)+strlen("_to_plot"));
	// char *str = strcat(strcpy(file_to_plot, inst.input_file), "_to_plot");

	/***** TSP WITH SUBTOUR *****/
	if(!strncmp(inst.model_type, "subtour", 7))
	{
		if(TSPopt(&inst))
			print_error(" error within TSPopt()");
	}

	/***** TSP SEC_LOOP *****/
	if(!strncmp(inst.model_type, "sec_loop", 8))
	{
		if(TSPopt_sec_loop(&inst))
			print_error(" error within TSPopt()");
	}

	/***** TSP MTZ ******/
	if(!strncmp(inst.model_type, "mtz", 3))
	{
		if(TSPopt_mtz(&inst))
			print_error(" error within TSPopt_mtz()");
	}
	
	
	/***** TSP COMPACT CUSTOM *****/
	if(!strncmp(inst.model_type, "compact_custom", 14))
	{
		if(TSPopt_compact_custom(&inst))
			print_error(" error within TSPopt_compact_custom()");
	}

	print_plot(&inst, "plot_file.txt");

	free_instance(&inst);

	return 0;
}
