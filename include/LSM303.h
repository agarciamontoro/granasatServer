#ifndef LSM303_H__
#define LSM303_H__

// C standard libraries
#include <stdio.h>				// I/O: printf, fprintf...
#include <fcntl.h>				// File management, constants and declarations
#include <errno.h>				// Error constants
#include <stdlib.h>				// General functions: exit, atoi, rand, malloc, free...
#include <stdint.h>				// Standard int data-types: uint8_t
#include "i2c-dev.h"			// I2C management

// Program libraries
#include "sync_control.h"		// Timestamp management and synchronisation control


// register addresses
#define MAG_ADDRESS            (0x3C >> 1)
#define ACC_ADDRESS            (0x32 >> 1)
#define ACC_ADDRESS_SA0_A_LOW  (0x30 >> 1)
#define ACC_ADDRESS_SA0_A_HIGH (0x32 >> 1)

#define LSM303_CTRL_REG1_A       0x20
#define LSM303_CTRL_REG2_A       0x21
#define LSM303_CTRL_REG3_A       0x22
#define LSM303_CTRL_REG4_A       0x23
#define LSM303_CTRL_REG5_A       0x24
#define LSM303_CTRL_REG6_A       0x25 // DLHC only
#define LSM303_HP_FILTER_RESET_A 0x25 // DLH, DLM only
#define LSM303_REFERENCE_A       0x26
#define LSM303_STATUS_REG_A      0x27

#define LSM303_OUT_X_L_A         0x28
#define LSM303_OUT_X_H_A         0x29
#define LSM303_OUT_Y_L_A         0x2A
#define LSM303_OUT_Y_H_A         0x2B
#define LSM303_OUT_Z_L_A         0x2C
#define LSM303_OUT_Z_H_A         0x2D

#define LSM303_FIFO_CTRL_REG_A   0x2E // DLHC only
#define LSM303_FIFO_SRC_REG_A    0x2F // DLHC only

#define LSM303_INT1_CFG_A        0x30
#define LSM303_INT1_SRC_A        0x31
#define LSM303_INT1_THS_A        0x32
#define LSM303_INT1_DURATION_A   0x33
#define LSM303_INT2_CFG_A        0x34
#define LSM303_INT2_SRC_A        0x35
#define LSM303_INT2_THS_A        0x36
#define LSM303_INT2_DURATION_A   0x37

#define LSM303_CLICK_CFG_A       0x38 // DLHC only
#define LSM303_CLICK_SRC_A       0x39 // DLHC only
#define LSM303_CLICK_THS_A       0x3A // DLHC only
#define LSM303_TIME_LIMIT_A      0x3B // DLHC only
#define LSM303_TIME_LATENCY_A    0x3C // DLHC only
#define LSM303_TIME_WINDOW_A     0x3D // DLHC only

#define LSM303_CRA_REG_M         0x00
#define LSM303_CRB_REG_M         0x01
#define LSM303_MR_REG_M          0x02

#define LSM303_OUT_X_H_M         0x03
#define LSM303_OUT_X_L_M         0x04
#define LSM303_OUT_Z_H_M	   0x05
#define LSM303_OUT_Z_L_M	   0x06
#define LSM303_OUT_Y_H_M	   0x07
#define LSM303_OUT_Y_L_M	   0x08

#define LSM303_SR_REG_M          0x09
#define LSM303_IRA_REG_M         0x0A
#define LSM303_IRB_REG_M         0x0B
#define LSM303_IRC_REG_M         0x0C

#define LSM303_TEMP_OUT_H_M      0x31 // DLHC only
#define LSM303_TEMP_OUT_L_M      0x32 // DLHC onlyevice type.

#define LSM303_SR_REG_M          0x09
#define LSM303_IRA_REG_M         0x0A
#define LSM303_IRB_REG_M         0x0B
#define LSM303_IRC_REG_M         0x0C

/////////////////////////////////////////////////////////////////////////////////////
//ADDED BY ALEJANDRO
#define X   0
#define Y   1
#define Z   2

//Gain value depends of FS and GN value (view at function enableLSM303() at sensor.c)
#define A_GAIN 0.004    	//[G/LSB] FS=10
#define M_XY_GAIN 1100   	//[LSB/Gauss] GN=001
#define M_Z_GAIN 980	//[LSB/Gauss] GN=001
#define T_GAIN 8	//[LSB/ºC]

#define TS 100 	//Sample period. 1000ms

void  readBlock(uint8_t command, uint8_t size, uint8_t *data);

void selectDevice(int file, int addr);

void readACC(uint8_t *a, struct timespec* timestamp);

void readMAG(uint8_t *m, struct timespec* timestamp);

void readTMP(uint8_t *t, struct timespec* timestamp);

void writeAccReg(uint8_t reg, uint8_t value);

void writeMagReg(uint8_t reg, uint8_t value);

void writeTmpReg(uint8_t reg, uint8_t value);

void enableLSM303();
/////////////////////////////////////////////////////////////////////////////////////

#endif