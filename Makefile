# ------------------------
#	Compiler
# ------------------------
CC = gcc

# -------------------------------------------------------------
#	Flags and stuff needed for compiling
# 	
#	-I <path> is needed to include cplex.h for example
# -------------------------------------------------------------
TARGET = main
FILES=$(wildcard *.c) #this function lists all .c files
OBJECTS=$(patsubst %.c, %.o, $(FILES)) #substitute file.c -> file.o
CPLEX_LOC = /opt/ibm/ILOG/CPLEX_Studio128/cplex/include/ilcplex/
CFLAGS = -I $(CPLEX_LOC)
RM1=rm -f


all: $(TARGET) 

$(TARGET):$(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS) 

%.o: %.c
	$(CC) $(CFLAGS) -c $< 
#$< expands to the first prerequisite of the current target in this case <file>.c 
#	$(CC) $(CFLAGS) -c $< -o $@ 

#$(TARGET): main.o utilities.o
#	$(CC) $(CFLAGS) main.o utilities.o -o main

#main.o: main.c
#	$(CC) -c main.c

#utilities.o: utilities.c
#	$(CC) -c utilities.c 

# ---------------------------------------
#	Set the cleanup 
# ---------------------------------------

clean:
	$(RM1) *.o 
	$(RM1) $(TARGET)
