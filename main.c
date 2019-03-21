#include "tsp.h"

/*   MAIN   */
int main(int argc, char **argv)
{
	instance inst;
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
	

	/***** TSP *****
	if(TSPopt(&inst))
		print_error(" error within TSPopt()");

	print_plot(&inst, "plot_file.txt");
	*/


	/***** TSP MTZ ******/
	if(TSPopt_mtz(&inst))
		print_error(" error within TSPopt_mtz()");
	
	print_plot_mtz(&inst, "plot_file_mtz.txt");
	

	/***** TSP COMPACT CUSTOM ****
	if(TSPopt_compact_custom(&inst))
		print_error(" error within TSPopt_compact_custom()");
	*/

	//free_instance(&inst);

	return 0;
}
