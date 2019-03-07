#include "tsp.h"

#define LINE_LENGTH 180
/*   PROTOTYPES   */
void free_instance(instance *inst);
void parse_command_line(int argc, char** argv, instance *inst);
void print_error(const char *err);
void print_plot(instance *inst, char *plot_file_name);
void read_input(instance *inst);
void build_model(instance *inst);//, CPXENVptr env, CPXLPptr lp);


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

	print_plot(&inst, "plot_file");

	void build_model(instance *inst);//, CPXENVptr env, CPXLPptr lp);

	free_instance(&inst);
	return 0;
}