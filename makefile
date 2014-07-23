##
# Makefile 
# Author: Mario Román
# From: https://github.com/M42/mp-tsp/blob/master/makefile
##

# Macros para identificar los directorios.
INCLUDE=./include
OBJ=./obj
SRC=./src
DOC=./doc
BIN=./bin

# Macros para identificar el ejecutable y los archivos objeto.
EXECUTABLE= $(BIN)/granasatServer
HEADERS= $(wildcard $(INCLUDE)/*.h)
SOURCES= $(wildcard $(SRC)/*.c)
OBJECTS= $(patsubst $(SRC)/%.c, $(OBJ)/%.o, $(SOURCES))

# Flags usados al compilar y enlazar
CC=gcc
INCLUDES=-I./include
FLAGS=-w -O0 -g3 -fmessage-length=0 `pkg-config --cflags --libs opencv` -export-dynamic

# El objetivo global es el ejecutable.
all: $(EXECUTABLE)

# Generación del ejecutable a partir de ficheros objeto.
$(EXECUTABLE): $(OBJECTS)
	$(CC) -o $@ $(OBJECTS) $(INCLUDES) $(FLAGS)

# Generación de ficheros objeto.
$(OBJ)/%.o: $(SRC)/%.c $(HEADERS)
	$(CC) -o $@ $(INCLUDES) -c $< $(FLAGS)




# Falsos objetivos para limpieza, documentación y generacion HTML.
clean:
	@rm $(OBJ)/*.o && echo "Borrados ficheros objeto."
	@rm $(EXECUTABLE) && echo "Borrado ejecutable."

.PHONY: clean documentacion tabla