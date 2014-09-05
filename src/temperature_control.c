#include "temperature_control.h"

int DS1621_fd = -1;
int DS18B20_fd = -1;
FILE* BCM2835_fd = NULL;

uint8_t* current_temperature = NULL;

int enableTempSensors(){
	current_temperature = malloc(TEMP_FM_SIZE);

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

	/*DIR * dir;
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
	}*/


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
	BCM2835_fd = fopen ("/sys/class/thermal/thermal_zone0/temp", "r");

	if(BCM2835_fd == NULL){
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int readTempSensor(enum TEMP_SENSOR_ID sensor, int32_t* ptr, struct timespec* timestamp){
	char buf[256]; //Datos
	char tmpData[16]; //Temp C * 1000
	uint8_t LSM_temp[2];
	int temp;

	FILE *temperatureFile;
	int T;

	switch(sensor){
		case TEMP_SENSOR_CAM:
			//read(DS18B20_fd, buf, 256);
			//strncpy(tmpData, strstr(buf, "t=") + 2, 5);
			//sscanf(tmpData, "%"SCNd16, ptr);
			*ptr = (int32_t) readTempInmC(1, 0x4d);
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
				*ptr = buf[1] == 128 ? (int32_t) buf[0]*1000 : (int32_t) buf[0]*1000+500;
			}
			break;

		case TEMP_SENSOR_MAG:
			//readTMP(LSM_temp, &timestamp);
			//*ptr = (int16_t) ((LSM_temp[1] | LSM_temp[0] << 8) >> 4)/T_GAIN;
			readTEMP_2(ptr, timestamp);
			*ptr /= T_GAIN;
			*ptr *= 1000;
			break;

		case TEMP_SENSOR_CPU:
			temperatureFile = fopen ("/sys/class/thermal/thermal_zone0/temp", "r");
			if (temperatureFile == NULL)
			  ; //print some message
			fscanf (temperatureFile, "%d", ptr);
			//*ptr = (int32_t) T;
			fclose (temperatureFile);
			break;
	}
}

float tempToFloat(int16_t* ptr){

}

void disableTempSensors(){
	if(DS1621_fd != -1)
		close(DS1621_fd);
	if(DS18B20_fd != -1)
		close(DS18B20_fd);
	if(BCM2835_fd != NULL)
		fclose(BCM2835_fd);

	free(current_temperature);
}

// Private functions
int exec(char *command, char *out, int out_len);
float c2f(float f);

int readTempInmC(int bus, TC74_t addy)
{
	#define C_LEN	20	//length of command
	#define B_LEN	3	//length of bus
	#define A_LEN	10	//length of addy
	#define PADD	6	//leave some extra space

	#define T_LEN	6	//Max length of temperature string

	char i2c_command[A_LEN+B_LEN+C_LEN+PADD];
	strncpy(i2c_command, "/usr/sbin/i2cget -y", 20);

	if (bus == 1) {
		strncat(i2c_command, " 1", B_LEN);
	}
	else if (bus == 0) {
		strncat(i2c_command, " 0", B_LEN);
	}
	else {
		fprintf(stderr, "Valid values for bus are [0,1] %d is invalid.\n", bus);
		return -1;
	}

	char addystr[A_LEN];
	int ret = snprintf(addystr, A_LEN, " 0x%x", addy);
	if (ret < 0) {
		fprintf(stderr, "Unable to generate command string.\n");
		return -2;
	}
	else if (ret > A_LEN) {
		fprintf(stderr, "%d characters were truncated.\n", ret-A_LEN);
	}
	strncat(i2c_command, addystr, A_LEN);
	//printf("%s\n", i2c_command);

	char temp[T_LEN];
	ret = exec(i2c_command, temp, T_LEN);
	if (ret < 0)
	{
		return -3;
	}
	//parse the temperature

	return 1000*strtol(temp, NULL, 16); //@todo This might not handle negative tempertatures well
}

/**
 * Execute the command on the shell and give the first out_len characters from the
 * first line of output to the caller by placing them in the caller allocated buffer.
 * @param  command command to run on the shell
 * @param  out     a caller allocated buffer of out_len length
 * @param  out_len the max number of characters to return
 * @return         0 on success, < 0 on error
 */
int exec(char *command, char *out, int out_len)
{
	FILE *fp;

	//Open the command for reading.
	fp = popen(command, "r");
	if (fp == NULL) {
		fprintf(stderr, "Failed to run command %s\n", command);
		return -1;
	}

	// Read the output a line at a time - output it.
	// while (fgets(path, sizeof(path)-1, fp) != NULL) {
	// 	printf("%s", path);
	// }

	//read the first line and send it back to the caller
	char *ret = fgets(out, out_len-1, fp);
	if (ret == NULL)
	{
		fprintf(stderr, "Unable to read output from command %s\n", command);
		return -2;
	}

	//close
	pclose(fp);

	return 0;
}