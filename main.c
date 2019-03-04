#include "tsp.h"

#define LINE_LENGTH 180
/*   PROTOTYPES   */
void parse_command_line(int argc, char** argv, instance *inst);
void print_error(const char *err);
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