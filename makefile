##
# Makefile 
# Author: Mario Román
# Adapted by: Alejandro García
# From: https://github.com/M42/mp-tsp/blob/master/makefile
##

# Macros for identifying directories
INCLUDE=include
OBJ=obj
SRC=src
DOC=doc
BIN=bin

# Macros for identifiying files.
EXECUTABLE= $(BIN)/granasatServer
HEADERS= $(wildcard $(INCLUDE)/*.h)
SOURCES= $(wildcard $(SRC)/*.c)
OBJECTS= $(patsubst $(SRC)/%.c, $(OBJ)/%.o, $(SOURCES))

# Macros for sending new files to remote system with scp
RECEIVER= pi@192.168.0.200:
GLOBAL_PATH= /home/pi/development/Final_v7
SEND=N

# Compilation and linking flags
CC=gcc
INCLUDES=-I./include `pkg-config --cflags opencv`
FLAGS=-O0 -g3 -fmessage-length=0 `pkg-config --libs opencv` -export-dynamic

# Global target is the final program
all: $(EXECUTABLE)

# Binary generation from .o files
$(EXECUTABLE): $(OBJECTS) | $(BIN)
	$(CC) -o $@ $(OBJECTS) $(INCLUDES) $(FLAGS)

# .o files generation
$(OBJ)/%.o: $(SRC)/%.c $(HEADERS) | $(OBJ)
		$(CC) -o $@ $(INCLUDES) -c $<
ifeq ($(SEND),Y)
	scp $< $(RECEIVER)$(GLOBAL_PATH)/$<
endif

# Creation of binary directory
$(BIN):
	mkdir -p $(BIN)

# Creation of objects directory
$(OBJ):
	mkdir -p $(OBJ)


# Phony targets for sending all files through scp, for cleaning and for generation of documentation.
.PHONY: sendall clean doc

sendall:
	scp $(SOURCES) $(RECEIVER)$(GLOBAL_PATH)/src/
	scp $(HEADERS) $(RECEIVER)$(GLOBAL_PATH)/include/

clean:
	@rm $(OBJ)/*.o && echo "$(OBJ)/*.o files deleted."
	@rm $(EXECUTABLE) && echo "$(EXECUTABLE) deleted."

doc:
	doxygen doxyfile
