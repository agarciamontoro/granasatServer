//// Programa para controlar el sensor DS1621
////
//// Este programa configurara el sensor DS1621 de forma
//// que se le lea la temperatura cada segundo.
////
//// Manuel Milla 2014

#ifndef DS_1621_H__
#define DS_1621_H__

int ds1621_setup();

int readTempSensor(int fd, signed char* highByte, unsigned char* lowByte);

#endif
