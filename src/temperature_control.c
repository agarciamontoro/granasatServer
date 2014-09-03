#include "temperature_control.h"

int DS1621_fd = -1;
int DS18B20_fd = -1;
FILE* BCM2835_fd = NULL;

int enableTempSensors(){
	/////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////    DS1621 - GEN    /////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////

	char *fileName = "/dev/i2c-1";								// Name of the port we will be using
	int  address = 0x48;										// Address of the SRF02 shifted right one bit
	unsigned char buf[10];										// Buffer for data being read/ written on the i2c bus

	if ((DS1621_fd = open(fileName, O_RDWR)) < 0) {					// Open port for reading and writing
		printMsg(stderr, DS1621, "Failed to open i2c port\n");
		return EXIT_FAILURE;
	}

	if (ioctl(DS1621_fd, I2C_SLAVE, address) < 0) {					// Set the port options and set the address of the device we wish to speak to
		printMsg(stderr, DS1621, "Unable to get bus access to talk to slave\n");
		return EXIT_FAILURE;
	}


	buf[0]= 0xee; // start convert
	if ((write(DS1621_fd, buf, 1)) != 1) {
		printMsg(stderr, DS1621, "Unable to write\n");
		return EXIT_FAILURE;
	}
	buf[0]= 0xac; // Access config
	buf[1]= 0x00;

	if ((write(DS1621_fd, buf, 2)) != 2) {
		printMsg(stderr, DS1621, "Unable to write\n");
		return EXIT_FAILURE;
	}


	/////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////    DS18B20 - CAM    ////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////

	DIR * dir;
	struct dirent *dirent;
	char dev [16];  //Dev Id
	char devPath [128]; //ruta
	char path [] = "/sys/bus/w1/devices";

	dir = opendir (path);

	if (dir != NULL){
		while ((dirent = readdir (dir)))
			if (dirent->d_type ==DT_LNK &&
			strstr(dirent->d_name, "28-") != NULL) { 
				//Los links de 1-wire empiezan en 28-
				strcpy(dev, dirent->d_name);
			}

		(void) closedir (dir);
	}
	else{
		perror ("Couldn't open the w1 devices directory");
		return EXIT_FAILURE;
	}

	sprintf(devPath, "%s/%s/w1_slave", path, dev);

	DS18B20_fd = open(devPath, O_RDONLY);

	if(DS18B20_fd == -1){
		perror("Nop open w1 device\n");
		return EXIT_FAILURE;
	}


	/////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////    LSM303 - MAG     ////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////
    
    /** @todo Enable LSM303 in any other way. This way makes enableTempSensors() dependant
     * of enable_LSM303();
     */
    if(LSM303_fd == -1){
    	printMsg(stderr, MAIN, "LSM303_fd es 1\n");
    	return EXIT_FAILURE;
    }
    else{
		//P.36 datasheet Enable termometer, minimun data output rate 15Hz
    	writeTmpReg(LSM303_CRA_REG_M, 0b10010000);
    }


	/////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////    BCM2835 - CPU    ////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////
	BCM2835_fd = fopen ("/sys/class/thermal/thermal_zone0/temp", "rt");

	if(BCM2835_fd == NULL){
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int readTempSensor(enum TEMP_SENSOR_ID sensor, int16_t* ptr){
	char buf[256]; //Datos
	char tmpData[16]; //Temp C * 1000
	struct timespec timestamp;
	uint8_t LSM_temp[2];

	switch(sensor){
		case TEMP_SENSOR_CAM:
			read(DS18B20_fd, buf, 256);
			strncpy(tmpData, strstr(buf, "t=") + 2, 5);
			sscanf(tmpData, "%"SCNd16, ptr);
			break;

		case TEMP_SENSOR_GEN:
			// Commands for performing a ranging
			buf[0] =0xaa;

			if ((write(DS1621_fd, buf, 1)) != 1) {								// Write commands to the i2c port
				printMsg(stderr, DS1621, "Error writing to i2c slave\n");
				return EXIT_FAILURE;
			}
			// This sleep waits for the ping to come back

			if (read(DS1621_fd, buf, 2) != 2) {								// Read back data into buf[]
				printMsg(stderr, DS1621, "Unable to read from slave\n");
				return EXIT_FAILURE;
			}
			else {
				*ptr = buf[1] == 128 ? buf[0]*1000 : buf[0]*1000+500;
			}
			break;

		case TEMP_SENSOR_MAG:
			readTMP(LSM_temp, &timestamp);
			*ptr = (int16_t)(LSM_temp[1] | LSM_temp[0] << 8) >> 4;
			break;

		case TEMP_SENSOR_CPU:
			fscanf (BCM2835_fd, "%"SCNd16, ptr);
			fseek(BCM2835_fd, 0, SEEK_SET);
			break;
	}
}

float tempToFloat(int16_t* ptr){

}