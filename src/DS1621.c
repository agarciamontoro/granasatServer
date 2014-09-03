//// Programa para controlar el sensor DS1621
////
//// Este programa configurara el sensor DS1621 de forma
//// que se le lea la temperatura cada segundo.
////
//// Manuel Milla 2014

#include "DS1621.h"

int fd  = -1;



int readDS1621Sensor(signed char* highByte, unsigned char* lowByte)
{
	//int fd;
	unsigned char buf[10];
	// Commands for performing a ranging
	buf[0] =0xaa;

	if ((write(fd, buf, 1)) != 1) {								// Write commands to the i2c port
		printMsg(stderr, DS1621, "Error writing to i2c slave\n");
		return EXIT_FAILURE;
	}
	// This sleep waits for the ping to come back

	if (read(fd, buf, 2) != 2) {								// Read back data into buf[]
		printMsg(stderr, DS1621, "Unable to read from slave\n");
		return EXIT_FAILURE;
	}
	else {
		*highByte = buf[0];
		*lowByte = buf[1];
	}

	return EXIT_SUCCESS;
}
