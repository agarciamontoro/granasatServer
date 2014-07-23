/*
 * sensors.h
 *
 *  Created on: 11/04/2014
 *      Author: alejandro
 */
#include <stdbool.h>
#include <stdio.h>
#include "connection.h"
#include "LSM303.h"
#include "DS1621.h"

#ifndef SENSORS_H_
#define SENSORS_H_

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