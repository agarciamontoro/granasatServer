/*
 * sensors.c
 *
 *  Created on: 11/04/2014
 *      Author: alejandro
 */

#include "sensors.h"

pid_t LED_PID = -1;
FILE* CPU_temp_file = NULL;

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
		
		if(readACC(accelerometer, &timestamp) == EXIT_SUCCESS){
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

	printMsg(stderr, LSM303, "ACC:\t%4.3f %4.3f %4.3f\n", accF[0],accF[1],accF[2]);	
    if(success)
        return EXIT_SUCCESS;
    else
        return EXIT_FAILURE;
}

int readAndStoreMagnetometer(FILE* file){
	int success = 0;
	uint8_t magnetometer[6];

	struct timespec timestamp;
	/////////////////////////////////////////////////////////////////////////////////////
	pthread_rwlock_wrlock( &magnetometer_rw_lock );
		if(readMAG(magnetometer, &timestamp) == EXIT_SUCCESS){
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
		else{
			return EXIT_FAILURE;
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

	printMsg(stderr, LSM303, "MAG: %4.3f %4.3f %4.3f\n", MAG[0],MAG[1],MAG[2]);

    if(success)
        return EXIT_SUCCESS;
    else
        return EXIT_FAILURE;
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

double readCPUtemperature(){
	int temperature;
	char line[80];

	double T;
	fscanf (CPU_temp_file, "%lf", &T);
	fseek(CPU_temp_file, 0, SEEK_SET);
	T /= 1000;

	return T;
}

int readAndStoreTemperatures(FILE* file){
	int32_t temperatures[4];
	struct timespec timestamp;

	readTempSensor(TEMP_SENSOR_CAM, &temperatures[0], NULL); //G
	readTempSensor(TEMP_SENSOR_GEN, &temperatures[1], NULL); //G
	readTempSensor(TEMP_SENSOR_MAG, &temperatures[2], &timestamp); //G
	readTempSensor(TEMP_SENSOR_CPU, &temperatures[3], NULL); //W

	pthread_rwlock_wrlock( &temperatures_rw_lock );
		int offset = 0;
		memcpy(current_temperature + offset, temperatures, 4*sizeof(int32_t));
		offset += sizeof(int32_t)*4;
		//Actual timestamp to shared buffer
		memcpy(current_temperature + offset, &(timestamp.tv_sec), TV_SEC_SIZE);
		offset += TV_SEC_SIZE;
		memcpy(current_temperature + offset, &(timestamp.tv_nsec), TV_NSEC_SIZE);
		offset += TV_NSEC_SIZE;
	pthread_rwlock_unlock( &temperatures_rw_lock );


	offset = 0;
	int i;
	for (i = 0; i < 4; ++i){
		temperatures[i] = (int32_t)(current_temperature[offset+0] | current_temperature[offset+1] << 8 | current_temperature[offset+2] << 16 | current_temperature[offset+3] << 24);
		offset += 4;
	}

	printMsg(stderr, MAIN, "Temperatures: %d - %d - %d - %d\n",
			 temperatures[0], temperatures[1], temperatures[2], temperatures[3]);

	return EXIT_SUCCESS;
}

int enableTemperatureSensors(){
	//Enable LSM303 sensor - Magnetometer/Accelerometer + Temperature 4
	enableLSM303();
	//Enable DS1621 sensor - Temperature 1
	//ds1621_setup();
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