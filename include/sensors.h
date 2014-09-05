/**
 * @file sensors.h
 * @author Alejandro García Montoro
 * @date 11 Apr 2014
 * @brief Sensor communication management. Declarations.
 *
 * @details sensors.h declares global variables and provides the user
 * with the needed functions to communicate with the sensors, , manage the synchronisation in GranaSAT server.
 * It includes semaphores to control the access to the camera buffers and
 * to control image processing parameters. Furthermore, it provides variables
 * and functions to work with timestamps.
 *
 */

#ifndef SENSORS_H_
#define SENSORS_H_

// C standard libraries
#include <stdbool.h>				// Needed for bool datatype. TODO: Remove all bool variables and replace them with ints
#include <stdio.h>					// I/O: printf, fprintf...
#include <unistd.h>					// System call functions: read, write, close...
#include <signal.h>					// Signal management

// Program libraries
#include "connection.h"				// Connection management library
#include "LSM303.h"					// Magnetomere-accelerometer library
#include "DS1621.h"					// Temperature sensor library
#include "DMK41BU02.h"				// Camera management library
#include "sync_control.h"		// Timestamp management and synchronisation control
#include "temperature_control.h"


void readAndSendMagnetometer(int socket);

void readAndSendAccelerometer(int socket);

int readAndStoreAccelerometer(FILE* file);

int readAndStoreMagnetometer(FILE* file);

void enable_CPUtemperature();

double read_CPUtemperature();

int readAndStoreTemperatures(FILE* file);

int enableTemperatureSensors();

int disableTemperatureSensors();

#endif /* SENSORS_H_ */