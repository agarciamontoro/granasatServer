#ifndef CONNECTION_H__
#define CONNECTION_H__

//TODO: Implement a 'char* cmdToStr(int cmd)' function in order to have a readable log

// C standard libraries
#include <stdio.h>			// Input-output. printf, fprintf...
#include <string.h>			// String management
#include <stdlib.h>			// General functions: atoi, rand, malloc, free...
#include <unistd.h>			// System call functions: read, write, close...
#include <netinet/in.h>		// Socket constants and data-types
#include <errno.h>			// Error constants

#include "protocol.h"		// Connection protocol, shared with granasatClient
#include "sync_control.h"	// Timestamp management and synchronisation control
#include "DMK41BU02.h"		// Camera management library

extern int CONNECTED;

extern int SOCKET_COMMANDS;
extern int SOCKET_BIG;
extern int SOCKET_SMALL;
extern int LISTEN_SOCKET;

struct communication{
	int sockfd;
	int new_sockfd;
	int portno;
};

void error(const char *msg, int status);

int prepareSocket(int portno);

int connectToSocket(int sockfd);

char getCommand(int sockfd);

int getData(int sockfd, void* ptr, int n_bytes);

int sendData(int sockfd, void* ptr, int n_bytes);

void sendImage(int sockfd);

void sendAcc(int sockfd);

void sendMag(int sockfd);

void sendAccAndMag(int sockfd);

#endif