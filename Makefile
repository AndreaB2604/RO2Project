# ------------------------
#	Compiler
# ------------------------
CC=gcc

# -------------------------------------------------------------
#	Flags and stuff needed for compiling
# 	
#	-I <path> is needed to include cplex.h for example
# -------------------------------------------------------------
CFLAGS=-I /opt/ibm/ILOG/CPLEX_Studio128/cplex/include/ilcplex/


all: main

main: main.c
	$(CC) $(CFLAGS) main.c -o main

# ---------------------------------------
#	Set the cleanup 
# ---------------------------------------

clean:
	rm -f main *.o 
