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

#define TEMP_FM_SIZE	( sizeof(int32_t)*4 + TIMESTAMP_SIZE)

extern uint8_t* current_temperature;

enum TEMP_SENSOR_ID{
	TEMP_SENSOR_CAM,
	TEMP_SENSOR_GEN,
	TEMP_SENSOR_MAG,
	TEMP_SENSOR_CPU
};

typedef enum {
	TC74A0 = 0x48,
	TC74A1 = 0x49,
	TC74A2 = 0x4a,
	TC74A3 = 0x4b,
	TC74A4 = 0x4c,
	TC74A5 = 0x4d,
	TC74A6 = 0x4e,
	TC74A7 = 0x4f
} TC74_t;

int enableTempSensors();
int readTempSensor(enum TEMP_SENSOR_ID sensor, int32_t* ptr, struct timespec* timestamp);
float tempToFloat(int16_t* ptr);
void disableTempSensors();

/**
 * Reads the temperature from the sensor at address on the given bus.
 * Accurate to plus or minus 2 deg C.
 * @param  bus  0 or 1: RPIv2 bus 1 is broken out on P1 header
 * @param  addy address of a TC74 sensor
 * @return      temperature in Celsius
 */
int readTempInmC(int bus, TC74_t addy);

#endif