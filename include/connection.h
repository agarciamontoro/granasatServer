#ifndef CONNECTION_H__
#define CONNECTION_H__

// C standard libraries
#include <stdio.h>			// Input-output. printf, fprintf...
#include <string.h>			// String management
#include <stdlib.h>			// General functions: atoi, rand, malloc, free...
#include <unistd.h>			// System call functions: read, write, close...
#include <netinet/in.h>		// Socket constants and data-types
#include <errno.h>			// Error constants

#include "sync_control.h"	// Timestamp management and synchronisation control
#include "DMK41BU02.h"		// Camera management library

static const int CAPTURE_RATE_NSEC = 2000000000;
static int CONNECTED = 0;

struct communication{
	int sockfd;
	int new_sockfd;
	int portno;
};

void error(char *msg);

int getData(int sockfd);

char getCommand(int sockfd);

int getInt(int sockfd);

void sendData(int x, int sockfd);

void enableConnection(int* sockfd, int portno, int* clilen, struct sockaddr_in* cli_addr );

void sendData_v2(int sockfd, void* ptr, int n_bytes);

void sendImage(int sockfd, uint8_t* image_stream);

void sendAcc(int sockfd);

void sendMag(int sockfd);

void sendAccAndMag(int sockfd);

void createCommChannel(struct communication* comm);

int openSocket(int portno);


#endif