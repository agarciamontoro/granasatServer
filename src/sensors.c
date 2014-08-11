/*
 * sensors.c
 *
 *  Created on: 11/04/2014
 *      Author: alejandro
 */

#include "sensors.h"

pid_t LED_PID = -1;

void readAndSendTemperature(int socket, int fd){
	signed char highByte = -10;
	unsigned char lowByte = 5;
	int n, bytes_sent, total_bytes;
	bytes_sent = 0;
	total_bytes = sizeof(signed char);

	readTempSensor(fd, &highByte, &lowByte);

	printMsg(stderr, SENSORS, "Processing temperature\n");

	while (bytes_sent < total_bytes) {
		if ((n = write(socket, &highByte, total_bytes - bytes_sent)) < 0)
			error("ERROR writing to socket", 0);
		else
			bytes_sent += n;
	}

	total_bytes = sizeof(unsigned char);
	bytes_sent = 0;

	while (bytes_sent < total_bytes) {
		if ((n = write(socket, &lowByte, total_bytes - bytes_sent)) < 0)
			error("ERROR writing to socket", 0);
		else
			bytes_sent += n;
	}
}

void readAndSendMagnetometer(int socket){
	uint8_t magnetometer[6];
	int n, bytes_sent, total_bytes;
	bytes_sent = 0;
	total_bytes = 6;

	struct timespec timestamp;

	/////////////////////////////////////////////////////////////////////////////////////
	readMAG(magnetometer, &timestamp);
	/////////////////////////////////////////////////////////////////////////////////////

	int16_t m[3];
	float MAG[3];

	*m = (int16_t)(magnetometer[1] | magnetometer[0] << 8);
	*(m+1) = (int16_t)(magnetometer[5] | magnetometer[4] << 8);
	*(m+2) = (int16_t)(magnetometer[3] | magnetometer[2] << 8);

	*(MAG+0) = (float) *(m+0)/M_XY_GAIN;
	*(MAG+1) = (float) *(m+1)/M_XY_GAIN;
	*(MAG+2) = (float) *(m+2)/M_Z_GAIN;

	printMsg(stderr, LSM303, "Processing magnetometer: %4.3f %4.3f %4.3f\n", MAG[0],MAG[1],MAG[2]);
	sendData(socket, magnetometer, sizeof(*magnetometer)*6);
}

void readAndSendAccelerometer(int socket){
	uint8_t accelerometer[6];
	int n, bytes_sent, total_bytes;
	bytes_sent = 0;
	total_bytes = 6;

	struct timespec timestamp;
	/////////////////////////////////////////////////////////////////////////////////////
	readACC(accelerometer, &timestamp);
	/////////////////////////////////////////////////////////////////////////////////////

	float accF[3];
	*(accF+0) = (float) *(accelerometer+0)*A_GAIN;
	*(accF+1) = (float) *(accelerometer+1)*A_GAIN;
	*(accF+2) = (float) *(accelerometer+2)*A_GAIN;

	printMsg(stderr, LSM303, "Processing accelerometer: %4.3f %4.3f %4.3f\n", accF[0],accF[1],accF[2]);
	sendData(socket, accelerometer, sizeof(*accelerometer)*6);
}

void readAndStoreAccelerometer(FILE* file){
	uint8_t accelerometer[6];

	struct timespec timestamp;
	/////////////////////////////////////////////////////////////////////////////////////
	readACC(accelerometer, &timestamp);
	/////////////////////////////////////////////////////////////////////////////////////

	/*
	fwrite(&(timestamp.tv_sec), sizeof(timestamp.tv_sec), 1, file);
	fwrite(&(timestamp.tv_nsec), sizeof(timestamp.tv_nsec), 1, file);
	fwrite(accelerometer, 6*sizeof(unsigned char), 1, file);
	*/
	/*
	int16_t acc[3];
	*acc = (int16_t)(accelerometer[0] | accelerometer[1] << 8) >> 4;
	*(acc+1) = (int16_t)(accelerometer[2] | accelerometer[3] << 8) >> 4;
	*(acc+2) = (int16_t)(accelerometer[4] | accelerometer[5] << 8) >> 4;
	*/

	fwrite(accelerometer, sizeof(*accelerometer), 6, file);

	float accF[3];
	*(accF+0) = (float) *(accelerometer+0)*A_GAIN;
	*(accF+1) = (float) *(accelerometer+1)*A_GAIN;
	*(accF+2) = (float) *(accelerometer+2)*A_GAIN;
	printMsg(stderr, LSM303, "%4.3f %4.3f %4.3f\n", accF[0],accF[1],accF[2]);
    /*fprintf(file, "%d %ld # %4.3f %4.3f %4.3f\n", (int)timestamp.tv_sec, timestamp.tv_nsec, accF[0],accF[1],accF[2]);*/
	
}

void readAndStoreMagnetometer(FILE* file){
	uint8_t magnetometer[6];

	struct timespec timestamp;
	/////////////////////////////////////////////////////////////////////////////////////
	readMAG(magnetometer, &timestamp);
	/////////////////////////////////////////////////////////////////////////////////////

	fwrite(magnetometer, sizeof(*magnetometer), 6, file);

	int16_t m[3];
	float MAG[3];

	*m = (int16_t)(magnetometer[1] | magnetometer[0] << 8);
	*(m+1) = (int16_t)(magnetometer[5] | magnetometer[4] << 8);
	*(m+2) = (int16_t)(magnetometer[3] | magnetometer[2] << 8);

	*(MAG+0) = (float) *(m+0)/M_XY_GAIN;
	*(MAG+1) = (float) *(m+1)/M_XY_GAIN;
	*(MAG+2) = (float) *(m+2)/M_Z_GAIN;

	printMsg(stderr, LSM303, "%4.3f %4.3f %4.3f\n", MAG[0],MAG[1],MAG[2]);
	/*fprintf(file, "%d %ld # %4.3f %4.3f %4.3f\n", (int)timestamp.tv_sec, timestamp.tv_nsec, MAG[0],MAG[1],MAG[2]);	*/
}

int setGPIOValue(int GPIO_number, bool on){
	if(on){
		LED_PID = fork();

		if (LED_PID == 0){
			execl("/home/pi/development/PJ_RPI/Examples/Blink/Source/Build/Blink", "Blink", (char*) NULL);
			return 0;
		}
		else
			printMsg(stderr, SENSORS, "Fork with PID: %d\n",LED_PID);

	}
	else
		kill(LED_PID, SIGTERM);

	return 0;
}

int readCPUtemperature(){
	FILE *fr;            /* declare the file pointer */
	int temperature;
	char line[80];

	fr = fopen ("/sys/class/thermal/thermal_zone0/temp", "rt");  /* open the file for reading */

	while(fgets(line, 80, fr) != NULL)
	{
		/* get a line, up to 80 chars from fr.  done if NULL */
		sscanf (line, "%d", &temperature);
	}
	fclose(fr);  /* close the file prior to exiting the routine */
	return temperature;
}

void readAndSendCPUTemperature(int newsockfd){
	int CPU_temp = readCPUtemperature();
	sendData(newsockfd, &CPU_temp, sizeof(CPU_temp));
}
