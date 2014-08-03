/*
 * sensors.c
 *
 *  Created on: 11/04/2014
 *      Author: alejandro
 */

#include "sensors.h"

pid_t LED_PID = -1;
int keep_running = 1;

void readAndSendImage(int sock){
	char file_to_read[30];
	FILE* raw_image;
	static int count = -1;

	printMsg(stderr, SENSORS, "Processing image\n");

	while(LAST_IMG_SAVED < 1){} //Waits for the image producer to have at least one image written in the SD

	count++;

	if(count > LAST_IMG_SAVED-1){
		count = LAST_IMG_SAVED - 1;
	}

	sprintf(file_to_read, "raw_image_%05d.data", count);

	//---- open the image to be sent ----
	raw_image = fopen(file_to_read, "r");

	if(raw_image == NULL){
		printMsg(stderr, SENSORS, "Error opening file %s: %s.\n", file_to_read, strerror(errno));
		//TODO: HANDLING ERRORS
		return;
	}
	else{
		printMsg(stderr, SENSORS, "File open: %s.\n", file_to_read);
	}

	unsigned char image_stream[1280*960];

	//---- read the image to be sent ----
	int bytes_read = fread((unsigned char*) image_stream, 1, 1280*960, raw_image);
	printMsg(stderr, SENSORS, "%d bytes read\n", bytes_read);

	//---- send the image ----
	int bytes_sent = sendImage_old(sock, image_stream);
	printMsg(stderr, SENSORS, "%d bytes sent\n\n", bytes_sent);

	fclose(raw_image);
}

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
			error("ERROR writing to socket");
		else
			bytes_sent += n;
	}

	total_bytes = sizeof(unsigned char);
	bytes_sent = 0;

	while (bytes_sent < total_bytes) {
		if ((n = write(socket, &lowByte, total_bytes - bytes_sent)) < 0)
			error("ERROR writing to socket");
		else
			bytes_sent += n;
	}
}

void readAndSendMagnetometer(int socket){
	unsigned char magnetometer[6];
	int n, bytes_sent, total_bytes;
	bytes_sent = 0;
	total_bytes = 6;

	/////////////////////////////////////////////////////////////////////////////////////
	readMAG_v2(magnetometer);
	/////////////////////////////////////////////////////////////////////////////////////

	printMsg(stderr, SENSORS, "Processing magnetometer\n");

	while (bytes_sent < total_bytes) {
		if ((n = write(socket, magnetometer, total_bytes - bytes_sent)) < 0)
			error("ERROR writing to socket");
		else
			bytes_sent += n;
	}
}

void readAndSendAccelerometer(int socket){
	unsigned char accelerometer[6];
	int n, bytes_sent, total_bytes;
	bytes_sent = 0;
	total_bytes = 6;

	struct timespec timestamp;
	/////////////////////////////////////////////////////////////////////////////////////
	readACC_v2(accelerometer, &timestamp);
	/////////////////////////////////////////////////////////////////////////////////////

	printMsg(stderr, SENSORS, "Processing accelerometer\n");

	while (bytes_sent < total_bytes) {
		if ((n = write(socket, accelerometer, total_bytes - bytes_sent)) < 0)
			error("ERROR writing to socket");
		else
			bytes_sent += n;
	}
}

void readAndStoreAccelerometer(FILE* file){
	unsigned char accelerometer[6];

	struct timespec timestamp;
	/////////////////////////////////////////////////////////////////////////////////////
	readACC_v2(accelerometer, &timestamp);
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

	float accF[3];
	*(accF+0) = (float) *(accelerometer+0)*A_GAIN;
	*(accF+1) = (float) *(accelerometer+1)*A_GAIN;
	*(accF+2) = (float) *(accelerometer+2)*A_GAIN;
    fprintf(file, "%d %ld # %4.3f %4.3f %4.3f\n", (int)timestamp.tv_sec, timestamp.tv_nsec, accF[0],accF[1],accF[2]);
	
}

void readAndStoreMagnetometer(FILE* file){
	unsigned char magnetometer[6];

	struct timespec timestamp;
	/////////////////////////////////////////////////////////////////////////////////////
	readMAG_v3(magnetometer, &timestamp);
	/////////////////////////////////////////////////////////////////////////////////////

	int16_t m[3];
	float MAG[3];

	*m = (int16_t)(magnetometer[1] | magnetometer[0] << 8);
	*(m+1) = (int16_t)(magnetometer[5] | magnetometer[4] << 8);
	*(m+2) = (int16_t)(magnetometer[3] | magnetometer[2] << 8);

	*(MAG+0) = (float) *(m+0)/M_XY_GAIN;
	*(MAG+1) = (float) *(m+1)/M_XY_GAIN;
	*(MAG+2) = (float) *(m+2)/M_Z_GAIN;

	fprintf(file, "%d %ld # %4.3f %4.3f %4.3f\n", (int)timestamp.tv_sec, timestamp.tv_nsec, MAG[0],MAG[1],MAG[2]);	
}

int sendImage_old(int sockfd, unsigned char* image_data) {
	int n, bytes_sent, total_bytes;
	bytes_sent = 0;
	total_bytes = 1280 * 960;

	printMsg(stderr, SENSORS, "Sending image\n");

	while (bytes_sent < total_bytes) {
		if ((n = write(sockfd, image_data + bytes_sent, total_bytes - bytes_sent)) < 0)
			error("ERROR writing to socket");
		else
			bytes_sent += n;
	}

	return bytes_sent;
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
	sendData(CPU_temp, newsockfd);
}
