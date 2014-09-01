/*
 * sensors.c
 *
 *  Created on: 11/04/2014
 *      Author: alejandro
 */

#include "sensors.h"

pid_t LED_PID = -1;
FILE* CPU_temp_file = NULL;

void* current_temperature = NULL;

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
	
	int16_t a_raw[6];
	*a_raw = (int16_t)(accelerometer[0] | accelerometer[1] << 8) >> 4;
     *(a_raw+1) = (int16_t)(accelerometer[2] | accelerometer[3] << 8) >> 4;
     *(a_raw+2) = (int16_t)(accelerometer[4] | accelerometer[5] << 8) >> 4;

	float accF[3];
	*(accF+0) = (float) *(a_raw+0)*A_GAIN;
	*(accF+1) = (float) *(a_raw+1)*A_GAIN;
	*(accF+2) = (float) *(a_raw+2)*A_GAIN;

	printMsg(stderr, LSM303, "Processing accelerometer: %4.3f %4.3f %4.3f\n", accF[0],accF[1],accF[2]);
	sendData(socket, accelerometer, sizeof(*accelerometer)*6);
}

int readAndStoreAccelerometer(FILE* file){
	int success = 0;
	uint8_t accelerometer[6];

	struct timespec timestamp;
	/////////////////////////////////////////////////////////////////////////////////////
	pthread_rwlock_wrlock( &accelerometer_rw_lock );
		
		if(readACC(accelerometer, &timestamp)){
			success = ( ( fwrite(accelerometer, sizeof(*accelerometer), 6, file) == 6 )
						&&
						( fwrite(&(timestamp.tv_sec), 1, TV_SEC_SIZE, file) == TV_SEC_SIZE )
						&&
						( fwrite(&(timestamp.tv_nsec), 1, TV_NSEC_SIZE, file) == TV_NSEC_SIZE )
					  );
			

		
			/**
			* The following function appears to be useless, but it is completely necessary.
			* See man 3 fopen to read the following paragraph, where this issue is explained:
			* Reads  and  writes  may be intermixed on read/write streams in any order.
			* Note that ANSI C requires that a file positioning function intervene between
			* output and input, unless an input operation encounters end-of-file.
			* (If this condition is not met, then a read is allowed  to  return  the  result  of
			* writes other than the most recent.)  Therefore it is good practice (and indeed
			*sometimes necessary under Linux) to put an fseek(3) or fgetpos(3) operation
			* between write and read operations on such a stream.  This operation may be an
			* apparent no-op (as in fseek(..., 0L, SEEK_CUR) called for  its  synchronizing
			* side effect.
			*/
			fseek(file, 0, SEEK_CUR);
		}
	pthread_rwlock_unlock( &accelerometer_rw_lock );
	/////////////////////////////////////////////////////////////////////////////////////

	int16_t a_raw[6];
	*a_raw = (int16_t)(accelerometer[0] | accelerometer[1] << 8) >> 4;
     *(a_raw+1) = (int16_t)(accelerometer[2] | accelerometer[3] << 8) >> 4;
     *(a_raw+2) = (int16_t)(accelerometer[4] | accelerometer[5] << 8) >> 4;

	float accF[3];
	*(accF+0) = (float) *(a_raw+0)*A_GAIN;
	*(accF+1) = (float) *(a_raw+1)*A_GAIN;
	*(accF+2) = (float) *(a_raw+2)*A_GAIN;

	//printMsg(stderr, LSM303, "ACC:\t%4.3f %4.3f %4.3f\n", accF[0],accF[1],accF[2]);	
	return success;
}

int readAndStoreMagnetometer(FILE* file){
	int success = 0;
	uint8_t magnetometer[6];

	struct timespec timestamp;
	/////////////////////////////////////////////////////////////////////////////////////
	pthread_rwlock_wrlock( &magnetometer_rw_lock );
		if(readMAG(magnetometer, &timestamp)){
			success = ( ( fwrite(magnetometer, sizeof(*magnetometer), 6, file) == 6 )
						&&
						( fwrite(&(timestamp.tv_sec), 1, TV_SEC_SIZE, file) == TV_SEC_SIZE )
						&&
						( fwrite(&(timestamp.tv_nsec), 1, TV_NSEC_SIZE, file) == TV_NSEC_SIZE )
					  );
		
			/**
			* The following function appears to be useless, but it is completely necessary.
			* See man 3 fopen to read the following paragraph, where this issue is explained:
			* Reads  and  writes  may be intermixed on read/write streams in any order.
			* Note that ANSI C requires that a file positioning function intervene between
			* output and input, unless an input operation encounters end-of-file.
			* (If this condition is not met, then a read is allowed  to  return  the  result  of
			* writes other than the most recent.)  Therefore it is good practice (and indeed
			*sometimes necessary under Linux) to put an fseek(3) or fgetpos(3) operation
			* between write and read operations on such a stream.  This operation may be an
			* apparent no-op (as in fseek(..., 0L, SEEK_CUR) called for  its  synchronizing
			* side effect.
			*/
			fseek(file, 0, SEEK_CUR);
		}
	pthread_rwlock_unlock( &magnetometer_rw_lock );
	/////////////////////////////////////////////////////////////////////////////////////

	int16_t m[3];
	*m = (int16_t)(magnetometer[1] | magnetometer[0] << 8);
    *(m+1) = (int16_t)(magnetometer[5] | magnetometer[4] << 8) ;
    *(m+2) = (int16_t)(magnetometer[3] | magnetometer[2] << 8) ;

	float MAG[3];
	*(MAG+0) = (float) *(m+0)/M_XY_GAIN;
	*(MAG+1) = (float) *(m+1)/M_XY_GAIN;
	*(MAG+2) = (float) *(m+2)/M_Z_GAIN;

	//printMsg(stderr, LSM303, "MAG: %4.3f %4.3f %4.3f\n", MAG[0],MAG[1],MAG[2]);
	return success;
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

void enable_CPUtemperature(){
	CPU_temp_file = fopen ("/sys/class/thermal/thermal_zone0/temp", "rt");  /* open the file for reading */
}

int readCPUtemperature(){
	int temperature;
	char line[80];

	while(fgets(line, 80, CPU_temp_file) != NULL)
	{
		/* get a line, up to 80 chars from fr.  done if NULL */
		sscanf (line, "%d", &temperature);
	}
	return temperature;
}

int readAndStoreTemperatures(FILE* file){
	signed char DS1621_high;
	unsigned char DS1621_low;
	int CPU_temp;
	uint8_t LSM303_temp[2];

	struct timespec timestamp;

	/*******************************************************
	***************** Reading temperatures *****************
	********************************************************/

	readDS1621Sensor(&DS1621_high, &DS1621_low);
	//readTC74sensor();
	CPU_temp = readCPUtemperature();
	readTMP(LSM303_temp, &timestamp);

	/*******************************************************
	*****************    Writing to file   *****************
	********************************************************/

	//Write temperatures
	fwrite(&DS1621_high, 1, sizeof(DS1621_high), file);
	fwrite(&DS1621_low, 1, sizeof(DS1621_low), file);
	//////fwrite(&TC74, 1, sizeof(TC74), file);
	fwrite(&CPU_temp, 1, sizeof(CPU_temp), file);
	fwrite(&LSM303_temp, 1, sizeof(*LSM303_temp)*2, file);

	//Write timestamp
	fwrite(&(timestamp.tv_sec), 1, TV_SEC_SIZE, file);
	fwrite(&(timestamp.tv_nsec), 1, TV_NSEC_SIZE, file);

	/*******************************************************
	*****************   Updating  buffer   *****************
	********************************************************/
	int offset = 0;

	pthread_rwlock_wrlock( &temperatures_rw_lock );
		//Actual DS1621 temperature to shared buffer
		memcpy(current_temperature + offset, &DS1621_high, sizeof(DS1621_high));
		offset += sizeof(DS1621_high);
		memcpy(current_temperature + offset, &DS1621_low, sizeof(DS1621_low));
		offset += sizeof(DS1621_low);

		//Actual TC74 temperature to shared buffer
		//memcpy(current_temperature + offset, &TC74, sizeof(TC74));
		//offset += sizeof(TC74);

		//Actual CPU temperature to shared buffer
		memcpy(current_temperature + offset, &CPU_temp, sizeof(CPU_temp));
		offset += sizeof(CPU_temp);

		//Actual LSM303 temperature to shared buffer
		memcpy(current_temperature + offset, &LSM303_temp, sizeof(*LSM303_temp)*2);
		offset += sizeof(*LSM303_temp)*2;


		//Actual timestamp to shared buffer
		memcpy(current_temperature + offset, &(timestamp.tv_sec), TV_SEC_SIZE);
		offset += TV_SEC_SIZE;
		memcpy(current_temperature + offset, &(timestamp.tv_nsec), TV_NSEC_SIZE);
		offset += TV_NSEC_SIZE;

		new_temp_send = 1;
	pthread_rwlock_unlock( &temperatures_rw_lock );

}

int enableTemperatureSensors(){
	current_temperature = malloc(TEMP_FILE_SIZE);
	//Enable LSM303 sensor - Magnetometer/Accelerometer + Temperature 4
	enableLSM303();
	//Enable DS1621 sensor - Temperature 1
	ds1621_setup();
	//Enable TC74 sensor - Temperature 2
	//TC74_setup(); /** @todo Code TC74 enable and reading functions
	//Enable CPU temperature sensor - Temperature 3
	enable_CPUtemperature();

	return EXIT_SUCCESS;
}

int disableTemperatureSensors(){
	free(current_temperature);

	return EXIT_SUCCESS;
}