//// Programa para controlar el sensor DS1621
////
//// Este programa configurara el sensor DS1621 de forma
//// que se le lea la temperatura cada segundo.
////
//// Manuel Milla 2014

#include "DS1621.h"

int ds1621_setup(){

	int fd;														// File descrition
	char *fileName = "/dev/i2c-1";								// Name of the port we will be using
	int  address = 0x48;										// Address of the SRF02 shifted right one bit
	unsigned char buf[10];										// Buffer for data being read/ written on the i2c bus

	if ((fd = open(fileName, O_RDWR)) < 0) {					// Open port for reading and writing
		printf("Failed to open i2c port\n");
		exit(1);
	}

	if (ioctl(fd, I2C_SLAVE, address) < 0) {					// Set the port options and set the address of the device we wish to speak to
		printf("Unable to get bus access to talk to slave\n");
		exit(1);
	}


	buf[0]= 0xee; // start convert
	if ((write(fd, buf, 1)) != 1) {
		printf("Unable to write\n");
		exit(1);
	}
	buf[0]= 0xac; // Access config
	buf[1]= 0x00;

	if ((write(fd, buf, 2)) != 2) {
		printf("Unable to wrie\n");
		exit(1);
	}


	return fd;

}

int readTempSensor(int fd, signed char* highByte, unsigned char* lowByte)
{
	//int fd;
	unsigned char buf[10];
	// Commands for performing a ranging
	buf[0] =0xaa;

	if ((write(fd, buf, 1)) != 1) {								// Write commands to the i2c port
		printf("Error writing to i2c slave\n");
		exit(1);
	}
	// This sleep waits for the ping to come back

	if (read(fd, buf, 2) != 2) {								// Read back data into buf[]
		printf("Unable to read from slave\n");
		exit(1);
	}
	else {
		*highByte = buf[0];
		*lowByte = buf[1];
	}

	return 1;
}
