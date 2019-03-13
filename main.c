#include "tsp.h"

#define LINE_LENGTH 180

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
	

	/***** TSP 
	if(TSPopt(&inst))
		print_error(" error within TSPopt()");

	print_plot(&inst, "plot_file.txt");
	*/

	/***** TSP COMPACT *****/
	if(TSPopt_compact(&inst))
		print_error(" error within TSPopt_compact()");
	/*

	print_plot(&inst, "plot_file_compact.txt");

	free_instance(&inst);
	*/
	return 0;
}
