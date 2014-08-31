/**
 * @file connection.h
 * @author Alejandro García Montoro
 * @date 27 Jul 2014
 * @brief Connection management. Declarations.
 *
 * @details connection.h declares global variables and provides the user
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
 *
 * <b>Data exchanged (SERVER > CLIENT) </b>
 *	-# <b>Image</b> (DMK41BU02 device): 1228828 B
 *		- Image data: 1280*960 px * 1 B/px = 1228800 B
 *		- Timestamp: sizeof(time_t) + sizeof(long) = 8 B
 *		- Parameters: sizeof(int) * 5 = 20
 *	-# <b>Magnetomer measurement</b> (LSM303 device): 14 B
 *		- Magnetometer data: 6 B
 *		- Timestamp: sizeof(time_t) + sizeof(long) = 8 B
 *	-# <b>Accelerometer measurement</b> (LSM303 device): 14 B
 *		- Magnetometer data: 6 B
 *		- Timestamp: sizeof(time_t) + sizeof(long) = 8 B
 *	-# <b>Temperature measurement</b> (DS1621 device): 10 B
 *		- Temperature data: sizeof(unsigned char) + sizeof(signed char) = 2 B
 *		- Timestamp: sizeof(time_t) + sizeof(long) = 8 B
 *	-# <b>Temperature measurement</b> (TC74 device): 9 B
 *		- Temperature data: sizeof(signed char) = 1 B
 *		- Timestamp: sizeof(time_t) + sizeof(long) = 8 B
 *	-# <b>Temperature measurement</b> (LSM303 device): 10 B
 *		- Temperature data: 2 B
 *		- Timestamp: sizeof(time_t) + sizeof(long) = 8 B
 *	-# <b>Temperature measurement</b> (BCM2835 device): 10 B
 *		- Temperature data: 2 B
 *		- Timestamp: sizeof(time_t) + sizeof(long) = 8 B

 * <b>Data exchanged (CLIENT > SERVER) </b>
 *	-# <b>Command</b>: 1 B
 *		- Command data: 1 B
 *	-# <b>Brightness value</b> (for device DMK41BU02): 4 B
 *		- Value: sizeof(int) = 4 B
 *	-# <b>Gamma value</b> (for device DMK41BU02): 4 B
 *		- Value: sizeof(int) = 4 B
 *	-# <b>Gain value</b> (for device DMK41BU02): 4 B
 *		- Value: sizeof(int) = 4 B
 *	-# <b>Exposition mode</b> (for device DMK41BU02): 4 B
 *		- Mode: sizeof(int) = 4 B
 *	-# <b>Exposition value</b> (for device DMK41BU02): 4 B
 *		- Value: sizeof(int) = 4 B
 *	-# <b>Brightness value</b> (for device DMK41BU02): 4 B
 *		- Value: sizeof(int) = 4 B
 *	-# <b>Number of centroids</b> (for star tracker algorithm): 4 B
 *		- Value: sizeof(int) = 4 B
 *	-# <b>Catalog</b> (for star tracker algorithm): 4 B
 *		- Value: sizeof(int) = 4 B
 *	-# <b>Pixel threshold</b> (for star tracker algorithm): 4 B
 *		- Value: sizeof(int) = 4 B
 *	-# <b>ROI size</b> (for star tracker algorithm): 4 B
 *		- Value: sizeof(int) = 4 B
 *	-# <b>Minimum points</b> (for star tracker algorithm): 4 B
 *		- Value: sizeof(int) = 4 B
 *	-# <b>Error</b> (for star tracker algorithm): 8 B
 *		- Value: sizeof(double) = 8 B
 *	-# <b>Binary threshold</b> (for horizon sensor algorithm): 4 B
 *		- Value: sizeof(int) = 4 B
 *	-# <b>Canny threshold</b> (for horizon sensor algorithm): 4 B
 *		- Value: sizeof(int) = 4 B

 *	

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
#include "LSM303.h"					// Magnetomere-accelerometer library

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////////////                    /////////////////////////////////////
////////////////////////////     VARIABLES      /////////////////////////////////////
////////////////////////////                    /////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Connection status
 *
 * @details CONNECTED is a boolean variable whose value is 1 when the connection with GranaSAT client
 * is set and ready and zero when this connection is lost. It is changed from getData() and sendData()
 * functions, as well as from the main.c control_connection() code, where it can be set to zero if the
 * server receives a ::MSG_END or a ::MSG_RESTART command from the client.
 * The error handling is basically based in this variable.
 *
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

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////////////                    /////////////////////////////////////
////////////////////////////     FUNCTIONS      /////////////////////////////////////
////////////////////////////                    /////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Print an error message to stderr and disconnect the server (if req.)
 *

 * @param msg Pointer to a string, whose content will be printed as the error message.
 * @param status Boolean variable: if 1, the variable ::CONNECTED is set to 0.

 * @return -

 * @note Set status to 1 can cause other functions to stop. It is a way to disconnect
 * the communication between the server and the client.
 * @note Set status to 1 if, and only if, you are really sure the connection does not work,
 * or you want to manually disconnect the communication.

 * @warning This function can change CONNECTED variable.
 */
void error(const char *msg, int status);


/**
 * @brief Set a socket to accept connections.
 *

 * @param portno Port number where the socket will be listening

 * @return Returns the file descriptor of a socket listening in port number @p portno.
 * If an error is encountered, a negative value is returned and an error message is
 * printed in stderr.

 * @see connectToSocket()
 * @see <a href="http://man7.org/linux/man-pages/man2/socket.2.html">man 2 socket</a>
 * @see <a href="http://man7.org/linux/man-pages/man2/bind.2.html">man 2 bind</a>
 * @see <a href="http://man7.org/linux/man-pages/man2/listen.2.html">man 2 listen</a>

 * @note The errors encountered can be caused by the following issues:
 * -# An error opening the socket.
 * -# An error binding the socket to the port @p portno
 * -# An error setting the socket to listen in the port @p portno.
 * @note This function is usually called before an infinite loop where
 * new connections are accepted with connectToSocket()
 */
int prepareSocket(int portno);

/**
 * @brief Accepts a new connection in the listening socket @p sockfd.
 *

 * @param sockfd Listening socket file descriptor.

 * @return Returns the file descriptor of the new accepted socket.
 * If an error is encountered, a negative value is returned and an error message is
 * printed in stderr.

 * @see prepareSocket()
 * @see <a href="http://man7.org/linux/man-pages/man2/accept.2.html">man 2 accept</a>

 * @note This function is actually a wrapper of <sys/socket.h> accept() function.
 * @warning connectToSocket() does not check the integrity of @p sockfd. It is the user's
 * responsability to pass a correctly opened listening socket file descriptor.
 */
int connectToSocket(int sockfd);

/**
 * @brief Reads a command from @p sockfd and returns it.

 * @param sockfd Socket file descriptor, enabled to read from it.

 * @return Returns the command received. If an error is encountered, ::MSG_PASS command
 * is returned.

 * @see protocol.h
 * @see getData()

 * @note Note that the return value can be ::MSG_PASS either if an error is encountered
 * or if ::MSG_PASS command is received.
 * @note This function is actually a wrapper of getData().
 *
 * @warning This function can change CONNECTED variable since it calls getData().
 */
char getCommand(int sockfd);

/**
 * @brief Reads data from a socket.

 * @param sockfd Socket file descriptor, enabled to read from it.
 * @param ptr Pointer to a buffer in which the data will be returned.
 * @param n_bytes Size of data to be received.

 * @return Returns the success of the function:
 * -# Returns 1 if the funtion succeded.
 * -# Returns 0 in any other case.

 * @see sendData()
 * @see <a href="http://man7.org/linux/man-pages/man2/recv.2.html">man 2 recv</a>

 * @note This function is actually a wrapper of <sys/socket.h> recv() function, with
 * flags MSG_NOSIGNAL and MSG_DONTWAIT set.

 * @warning This function can change CONNECTED variable. This can happen if the following
 * conditions occured:
 * -# recv() returns a negative a number.
 * -# recv() set errno to an error differente to EAGAIN.
 */
int getData(int sockfd, void* ptr, int n_bytes);

/**
 * @brief Sends data to a socket.

 * @param sockfd Socket file descriptor, enabled to write in it.
 * @param ptr Pointer to a buffer from which the data will be read.
 * @param n_bytes Size of data to be sent.

 * @return Returns the success of the function:
 * -# Returns 1 if the funtion succeded.
 * -# Returns 0 in any other case.

 * @see getData()
 * @see <a href="http://man7.org/linux/man-pages/man2/send.2.html">man 2 send</a>

 * @note This function is actually a wrapper of <sys/socket.h> send() function, with
 * flags MSG_NOSIGNAL and MSG_DONTWAIT set.

 * @warning This function can change CONNECTED variable. This can happen if the following
 * conditions occured:
 * -# send() returns a negative a number.
 * -# send() set errno to an error differente to EAGAIN.
 */
int sendData(int sockfd, void* ptr, int n_bytes);

/**
 * @brief Sends the buffered image to a socket.

 * @param sockfd Socket file descriptor, enabled to write in it.

 * @return Returns the success of the function:
 * -# Returns 1 if the funtion succeded.
 * -# Returns 0 in any other case. The failure can be caused because there is
 * no new image to be sent or because an error in sendData() call.

 * @see sendData()

 * @note 
 * @note This function is actually a wrapper of sendData() function.

 * @warning This function can change CONNECTED variable, since it calls sendData().
 */
int sendImage(int sockfd);

/**
 * @brief Sends accelerometer and magnetometer measurements to a socket.

 * @param mag_file FILE* variable, which points to a file of magnetometer measurements.
 * @param acc_file FILE* variable, which points to a file of accelerometer measurements.
 * @param sockfd Socket file descriptor, enabled to write in it.

 * @return Returns the success of the function:
 * -# Returns 1 if the funtion succeded.
 * -# Returns 0 in any other case.

 * @see sendData()

 * @note This function is actually a wrapper of sendData() function.

 * @warning This function can change CONNECTED variable, since it calls sendData().
 * @warning @p mag_file and @p acc_file should be ready file descriptors, since no file
 * error handling is done inside the function.).
 *
 */
int sendAccAndMag(FILE* mag_file, FILE* acc_file, int sockfd);

#endif