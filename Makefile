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
CPLEX_128 = /opt/ibm/ILOG/CPLEX_Studio128
CPLEX_129 = /opt/ibm/ILOG/CPLEX_Studio129
#CPLEX_LOC = /cplex/include/ilcplex
#LIB_LOC = /cplex/lib/x86-64_linux/static_pic
#LIBS = -L ${LIB_LOC} -lcplex -lm -lpthread -ldl 
#LIBS1 = -L ${LIB_LOC} -lcplex -lm 
#CFLAGS = -I $(CPLEX_LOC) 

ifneq "$(wildcard $(CPLEX_129) )" ""
	CPLEX_LOC = $(CPLEX_129)/cplex/include/ilcplex
	LIB_LOC = $(CPLEX_129)/cplex/lib/x86-64_linux/static_pic
else
	CPLEX_LOC = $(CPLEX_128)/cplex/include/ilcplex
	LIB_LOC = $(CPLEX_128)/cplex/lib/x86-64_linux/static_pic
endif

LIBS = -L ${LIB_LOC} -lcplex -lm -lpthread -ldl 
LIBS1 = -L ${LIB_LOC} -lcplex -lm 
CFLAGS = -I $(CPLEX_LOC)

RM1 = rm -f

all: $(TARGET) 

$(TARGET):$(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS) $(LIBS) 

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@  $(LIBS) 
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
	$(RM1) *.lp
	$(RM1) *.sav
	$(RM1) *.log
