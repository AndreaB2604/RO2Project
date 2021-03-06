#include "tsp.h"

/*   MAIN   */
int main(int argc, char **argv)
{
	instance inst;

	init_instance(&inst);
	
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

	/***** TSP SEC CALLBACK *****/
	if(!strncmp(inst.model_type, "sec_callback", 12))
	{
		if(TSPopt_sec_callback(&inst))
			print_error(" error within TSPopt_sec_callback()");
	}

	/***** TSP USER CALLBACK *****/
	if(!strncmp(inst.model_type, "usr_callback", 12))
	{
		if(TSPopt_usr_callback(&inst))
			print_error(" error within TSPopt_usr_callback()");
	}
	
	/***** TSP HARD-FIXING HEURISTIC *****/
	if(!strncmp(inst.model_type, "heur_hf", 7))
	{
		if(TSP_heur_hf(&inst))
			print_error(" error within TSP_heur_hf()");
	}

	/***** TSP LOCAL BRANCHING HEURISTIC *****/
	if(!strncmp(inst.model_type, "heur_lb", 7))
	{
		if(TSP_heur_lb(&inst))
			print_error(" error within TSP_heur_lb()");
	}

	/***** TSP NEAREST NEIGHBOURHOOD WITH GRASP HEURISTIC *****/
	if(!strncmp(inst.model_type, "heur_nn_grasp", 13))
	{
		if(TSP_nn_grasp_heur(&inst))
			print_error(" error within TSP_nn_grasp_heur()");
	}

	/***** TSP TWO-OPT HEURISTIC *****/
	if(!strncmp(inst.model_type, "heur_2opt", 9))
	{
		if(TSP_heur_2opt(&inst))
			print_error(" error within TSP_heur_2opt()");
	}

	/***** TSP VNS HEURISTIC *****/
	if(!strncmp(inst.model_type, "heur_vns", 8))
	{
		if(TSP_heur_vns(&inst))
			print_error(" error within TSP_heur_vns()");
	}

	/***** TSP TABU SEARCH HEURISTIC *****/
	if(!strncmp(inst.model_type, "heur_tabu", 9))
	{
		if(TSP_heur_tabu(&inst))
			print_error(" error within TSP_heur_tabu()");
	}

	/***** TSP MODULO HEURISTIC *****/
	if(!strncmp(inst.model_type, "modulo", 6))
	{
		if(TSP_modulo(&inst))
			print_error(" error within TSP_modulo()");
	}

	print_plot(&inst, "plot_file.txt");

	free_instance(&inst);

	return 0;
}
