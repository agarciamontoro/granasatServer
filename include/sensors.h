/**
 * @file sensors.h
 * @author Alejandro García Montoro
 * @date 11 Apr 2014
 * @brief Sensor communication for GranaSAT server.
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

#include <stdbool.h>
#include <stdio.h>
#include "connection.h"
#include "LSM303.h"
#include "DS1621.h"

extern int keep_running;

void readAndSendTemperature(int socket, int fd);

void readAndSendMagnetometer(int socket);

void readAndSendAccelerometer(int socket);

void readAndStoreAccelerometer(FILE* file);

void readAndStoreMagnetometer(FILE* file);

int sendImage_old(int sockfd, unsigned char* image_data);

void readAndSendImage(int sock);

int setGPIOValue(int GPIO_number, bool on);

int readCPUtemperature();

void readAndSendCPUTemperature(int newsockfd);

#endif /* SENSORS_H_ */