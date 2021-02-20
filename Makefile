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

SRC_DIR = src
BUILD_DIR = build
BIN_DIR = bin

CURRENT_DIR = $(shell pwd)

CPLEX_128 = /opt/ibm/ILOG/CPLEX_Studio128
CPLEX_129 = /opt/ibm/ILOG/CPLEX_Studio129
CPLEX_1210 = /opt/ibm/ILOG/CPLEX_Studio1210
CPLEX_201 = /opt/ibm/ILOG/CPLEX_Studio201
CPLEX_BLADE = /nfsd/rop/sw/ibm/cos128

SRC_FILES = $(wildcard $(SRC_DIR)/*.c) #this function lists all .c files
OBJ_FILES = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRC_FILES)) #substitute file.c -> file.o


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

COPT  = -m64 -fPIC
CPLEXLIB = cplex$(dynamic:yes=2010)
CONCORDELIB = ${CURRENT_DIR}/concorde
CPLEX_LIBS = -L ${LIB_LOC}  -l$(CPLEXLIB) -lm -lpthread -ldl
CONCORDE_LIBS = -L ${CONCORDELIB}/ -lconcorde

INCL = $(COPT) -I $(CPLEX_LOC) -I ${CONCORDELIB} #-g -Wall -Wpedantic -fsanitize=address #-Wextra

RM1 = rm -f

.PHONY: createdir all clean again

all: createdir $(TARGET) 

$(TARGET):$(OBJ_FILES)
	$(CC) $(INCL) -o $(TARGET) $(OBJ_FILES) $(CONCORDE_LIBS) $(CPLEX_LIBS) 

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(INCL) -c $< -o $@ $(CONCORDE_LIBS) $(CPLEX_LIBS) 

# ---------------------------------------
#	Set the cleanup 
# ---------------------------------------
clean:
	$(RM1) $(BUILD_DIR)/*.o
	$(RM1) *.o 
	$(RM1) $(TARGET)
	$(RM1) *.lp
	$(RM1) *.sav
	$(RM1) *.log
	$(RM1) *.txt

createdir:
	mkdir -p build

again:
	make clean
	make