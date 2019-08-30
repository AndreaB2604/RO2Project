# RO2Project

This repository contains all the code developed during the Opeations Research 2 at the Department of Information Engineering (DEI) at the Padova University. During thecourse we studied, developed, implemented and tested several techniques for the Traveling Salesman Problem. 

To compile the code:
* Install IBM ILOG CPLEX (if the version is different from 12.8 or 12.9, change the Makefile);
* (optional) Install concorde and link it in the Makefile (this step is needed to run the UserCutCallback code)
* Compile using `make`

To run the code use:

    ./main -(parameter) <value of the parameter> 

For example:

    ./main -f test_file/it16862.tsp -model heur_vns -time_limit 30

The available *parameters* as input can be:

    -file <file>
    -model <the model>
    -random_seed <random seed>
    -time_limit <time limit in seconds>

Use:

    ./main -help

to get more information or

    ./main -(parameter)

to get more information about the parameters

In the *report* folder, there is the report of our work in PDF format.
