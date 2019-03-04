# ------------------------
#	Compiler
# ------------------------
CC = gcc

# -------------------------------------------------------------
#	Flags and stuff needed for compiling
# 	
#	-I <path> is needed to include cplex.h for example
# -------------------------------------------------------------
CPLEX_LOC = /opt/ibm/ILOG/CPLEX_Studio128/cplex/include/ilcplex/
CFLAGS = -I $(CPLEX_LOC)


all: main

main: main.o utilities.o
	$(CC) $(CFLAGS) main.o utilities.o -o main

main.o: main.c
	$(CC) -c main.c

utilities.o: utilities.c
	$(CC) -c utilities.c 

# ---------------------------------------
#	Set the cleanup 
# ---------------------------------------

clean:
	rm -f main *.o 
