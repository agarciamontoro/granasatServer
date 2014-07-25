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
INCLUDES=-I./include `pkg-config --cflags opencv`
FLAGS=-O0 -g3 -fmessage-length=0 `pkg-config --libs opencv` -export-dynamic

# El objetivo global es el ejecutable.
all: $(EXECUTABLE)

# Generación del ejecutable a partir de ficheros objeto.
$(EXECUTABLE): $(OBJECTS) | $(BIN)
	$(CC) -o $@ $(OBJECTS) $(INCLUDES) $(FLAGS)

# Generación de ficheros objeto.
$(OBJ)/%.o: $(SRC)/%.c $(HEADERS) | $(OBJ)
	$(CC) -o $@ $(INCLUDES) -c $< && scp $< pi@192.168.0.200:/home/pi/development/Final_v4/src/

$(BIN):
	mkdir -p $(BIN)

$(OBJ):
	mkdir -p $(OBJ)


# Falsos objetivos para limpieza, documentación y generacion HTML.
clean:
	@rm $(OBJ)/*.o && echo "Borrados ficheros objeto."
	@rm $(EXECUTABLE) && echo "Borrado ejecutable."

send:
	scp $(SRC)/*.c pi@192.168.0.200:/home/pi/development/Final_v4/src/
	scp $(INCLUDE)/*.h pi@192.168.0.200:/home/pi/development/Final_v4/include/

.PHONY: clean documentacion tabla