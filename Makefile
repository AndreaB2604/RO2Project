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
CPLEX_1210 = /opt/ibm/ILOG/CPLEX_Studio1210
CPLEX_201 = /opt/ibm/ILOG/CPLEX_Studio201
CPLEX_BLADE = /nfsd/rop/sw/ibm/cos128
current_dir = $(shell pwd)
CONCORDELIB = ${current_dir}/concorde

CONCORDEDIR = concorde
CONCNAME = concorde.a

ifneq "$(wildcard $(CPLEX_BLADE) )" ""
	CPLEX_LOC = $(CPLEX_BLADE)/cplex/include/ilcplex
	LIB_LOC = $(CPLEX_BLADE)/cplex/lib/x86-64_linux/static_pic
else ifneq "$(wildcard $(CPLEX_201) )" ""
	CPLEX_LOC = $(CPLEX_201)/cplex/include/ilcplex
	LIB_LOC = $(CPLEX_201)/cplex/lib/x86-64_linux/static_pic
else ifneq "$(wildcard $(CPLEX_1210) )" ""
	CPLEX_LOC = $(CPLEX_1210)/cplex/include/ilcplex
	LIB_LOC = $(CPLEX_1210)/cplex/lib/x86-64_linux/static_pic
else ifneq "$(wildcard $(CPLEX_129) )" ""
	CPLEX_LOC = $(CPLEX_129)/cplex/include/ilcplex
	LIB_LOC = $(CPLEX_129)/cplex/lib/x86-64_linux/static_pic
else ifneq "$(wildcard $(CPLEX_128) )" ""
	CPLEX_LOC = $(CPLEX_128)/cplex/include/ilcplex
	LIB_LOC = $(CPLEX_128)/cplex/lib/x86-64_linux/static_pic
endif

LIBS = -L ${LIB_LOC}  -lcplex -lm -lpthread -ldl
CONCLIBS = -L ${CONCORDELIB}/ -lconcorde
LIBS1 = -L ${LIB_LOC} -lcplex -lm 
CFLAGS = -I $(CPLEX_LOC) -I ${CONCORDELIB} #-g -Wall -Wpedantic -fsanitize=address #-Wextra

RM1 = rm -f

all: $(TARGET) 

$(TARGET):$(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS) $(CONCLIBS) $(LIBS) 

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ $(CONCLIBS) $(LIBS) 

# ---------------------------------------
#	Set the cleanup 
# ---------------------------------------

clean:
	$(RM1) *.o 
	$(RM1) $(TARGET)
	$(RM1) *.lp
	$(RM1) *.sav
	$(RM1) *.log
	$(RM1) *.txt
