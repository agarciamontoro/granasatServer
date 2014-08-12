/**
 * @file connection.h
 * @author Alejandro García Montoro
 * @date 27 Jul 2014
 * @brief Connection management.
 *
 * @details connection.h declares declares global variables and provides the user
 * with some functions to manage the connection between GranaSAT server and GranaSAT client.
 * It includes variables to store the different socket file descriptors, as well as a variable
 * to control the connection losses and ensure that both the server and the client are connected.
 * 
 * The connection between the client and the server is divided in three channels, that are created
 * through three different ports. The communication channels are the following:
 *
 * -# A command socket, where the client write commands and the server reads them.
 * -# A big data socket, where the server writes the images taken from the camera and the client
 * receives them.
 * -# A small data socket, where the server writes the smallest packets of data and the client
 * receives them. Through this socket, the following data is exchanged:
 *		-# Magnetometer and accelerometer measurements.
 *		-# Temperature sensor measurements.
 *		-# Attitude measurements.
 * 
 * The connection error handling is now controlled only from getData() and sendData() functions.
 * It could be improved in the future.
 */

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

/**
 * @brief Connection status
 *
 * @details CONNECTED is a boolean variable whose value is 1 when the connection with GranaSAT client
 * is set and ready and zero when this connection is lost. It is changed from getData() and sendData()
 * functions, as well as from the main.c control_connection() code, where it can be set to zero if the
 * server receives a ::MSG_END or a ::MSG_RESTART command from the client.
 * It is initialised to zero.
 */
extern int CONNECTED;

/**
 * @brief Commands socket file descriptor
 *
 * @details SOCKET_COMMANDS stores the file descriptor for the commands socket. It is initialised to
 * zero and is set with the output of connectToSocket() function.
 */
extern int SOCKET_COMMANDS;

/**
 * @brief Big data socket file descriptor
 *
 * @details SOCKET_BIG stores the file descriptor for the big data socket. It is initialised to
 * zero and is set with the output of connectToSocket() function.
 */
extern int SOCKET_BIG;

/**
 * @brief Small data socket file descriptor
 *
 * @details SOCKET_SMALL stores the file descriptor for the small data socket. It is initialised to
 * zero and is set with the output of connectToSocket() function.
 */
extern int SOCKET_SMALL;


/**
 * @brief Listen commands socket file descriptor
 *
 * @details LISTEN_COMMANDS stores the file descriptor for the listening socket, where the command socket
 * is then accepted. This socket listens at port ::PORT_COMMANDS. It is initialised to zero and is set
 * with the output of prepareSocket() function.
 */
extern int LISTEN_COMMANDS;

/**
 * @brief Listen big data socket file descriptor
 *
 * @details LISTEN_BIG stores the file descriptor for the listening socket, where the big data socket
 * is then accepted. This socket listens at port ::PORT_BIG_DATA. It is initialised to zero and is set
 * with the output of prepareSocket() function.
 */
extern int LISTEN_BIG;

/**
 * @brief Listen small data socket file descriptor
 *
 * @details LISTEN_SMALL stores the file descriptor for the listening socket, where the small data socket
 * is then accepted. This socket listens at port ::PORT_SMALL_DATA. It is initialised to zero and is set
 * with the output of prepareSocket() function.
 */
extern int LISTEN_SMALL;

void error(const char *msg, int status);

int prepareSocket(int portno);

int connectToSocket(int sockfd);

char getCommand(int sockfd);

int getData(int sockfd, void* ptr, int n_bytes);

int sendData(int sockfd, void* ptr, int n_bytes);

int sendImage(int sockfd);

int sendAccAndMag(FILE* mag_file, FILE* acc_file, int sockfd);

#endif