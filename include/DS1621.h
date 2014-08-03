//// Programa para controlar el sensor DS1621
////
//// Este programa configurara el sensor DS1621 de forma
//// que se le lea la temperatura cada segundo.
////
//// Manuel Milla 2014s

#ifndef DS_1621_H__
#define DS_1621_H__

// C standard libraries
#include <stdio.h>				// I/O: printf, fprintf...
#include <stdlib.h>				// General functions: exit, atoi, rand, malloc, free...
#include <fcntl.h>				// File management, constants and declarations
#include "i2c-dev.h"			// I2C management

#include "sync_control.h"		// Timestamp management and synchronisation control

int ds1621_setup();

int readTempSensor(int fd, signed char* highByte, unsigned char* lowByte);

#endif
