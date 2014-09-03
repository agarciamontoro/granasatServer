#ifndef _TEMP_CONTROL_H_
#define _TEMP_CONTROL_H_

#include <stdint.h>
#include <stdio.h>					// I/O: printf, fprintf...
#include <unistd.h>					// System call functions: read, write, close...
#include <fcntl.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include "sync_control.h"			// Timestamp management and synchronisation control
#include "i2c-dev.h"				// I2C management
#include "LSM303.h"					// Magnetomere-accelerometer library
#include <inttypes.h>

#define TEMP_FM_SIZE	( sizeof(int16_t)*4 + TIMESTAMP_SIZE)

enum TEMP_SENSOR_ID{
	TEMP_SENSOR_CAM,
	TEMP_SENSOR_GEN,
	TEMP_SENSOR_MAG,
	TEMP_SENSOR_CPU
};

int enableTempSensors();
int readTempSensor(enum TEMP_SENSOR_ID sensor, int16_t* ptr);
float tempToFloat(int16_t* ptr);
int disableTempSensors();

#endif