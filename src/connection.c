/**
 * @file connection.c
 * @author Alejandro García Montoro
 * @date 27 Jul 2014
 * @brief Connection management. Definitions.
 */

 #include "connection.h"

int CONNECTED = 0;

int SOCKET_COMMANDS = 0;
int SOCKET_BIG = 0;
int SOCKET_SMALL = 0;

int LISTEN_COMMANDS = 0;
int LISTEN_BIG = 0;
int LISTEN_SMALL = 0;

/**
 * @details
 * error() prints an error message to stderr, labeled as CONNECTION, using printMsg() function.
 * Depending on @p status, error() forces the disconnection of all communication channels
 * by setting ::CONNECTED globar variable to 0. If this occurs, in addition to the error message
 * pointed by @p msg, another message is output to stderr, also labeled as CONNECTION, noticing
 * the disconnection.
 * 
 * This function is usually called after an error is detected from send() or recv() functions, as
 * well as from bind() or socket(). This function has to be called with @p status set to zero if,
 * and only if, the connection is intended to be finished, because an error is encountered or because
 * the user wants the connection to be manually finished.
 *
 * An example of use can be seen in prepareSocket():

 * @code
 * int sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (sockfd < 0){
	 error( "ERROR opening socket", 0 );
	}
	else{
		//CODE TO BE EXECUTED IN SUCCESS.
	}
 * @endcode
 *
 * @todo Let error() accepts a random number of arguments, in order to implement a printf-like function.
 *
 * <b> General behaviour </b> @n
 * The steps performed by error() are the following:
 */
void error(const char *msg, int status) {
	char error_string[75];

	/**
	*	@details -# Backup of errno value in error_string string.
	*/
	strerror_r(errno, error_string, 75);

	/**
	* 	@details -# Printing of the error message, which is done after concatenate the
	* message pointed by @p msg with the output string from strerror_r().
	*/
	printMsg(stderr, CONNECTION, "%s%s: %s.\n", KLRE, msg, error_string);
	
	/**
	*	@details
	*	-# If status equals to 1:
	*		-# ::CONNECTED variable is set to zero
	*		-# Another message noticing the disconnection is printed in stderr.
	*/
	if(status){
		printMsg(stderr, CONNECTION, "DISCONNECTING.\n");
		CONNECTED = 0;
	}
}

/**
 * @details
 * prepareSocket() receives a port number (@p portno), in which a newly opened socket
 * listens to accept new connections. If the function succeeds, the file descriptor of
 * this socket is returned.
 * Some information is logged using printMsg() function, labelled as ::CONNECTION.
 *
 * This function is never used alone, as it is intended to open a socket in which new connections
 * will be accepted. Since it returns the file descriptor of the listening socket, this value
 * shall be used as the single parameter of connectToSocket().
 *
 * The following piece of code is self-explanatory:

 * @code
	int portno = 51717;
	int LISTEN_SOCKFD, SOCKFD;

	LISTEN_SOCKFD = prepareSocket(portno);

	if (LISTEN_SOCKFD < 0){
	 error( "ERROR opening listening socket", 0 );
	}
	else{
		SOCKFD = connectToSocket(LISTEN_SOCKFD);

		//ERROR HANDLING AND COMMUNICATION CODE IF SUCCESS
	}
 * @endcode

 * <b> General behaviour </b> @n
 * The steps performed by prepareSocket() are the following:
 */
int prepareSocket(int portno){
	int sockfd, newsockfd;
	struct sockaddr_in serv_addr;
	int n;
	int data;

	printMsg(stderr, CONNECTION, "Using port #%d\n", portno );

	/**
	*	@details -# Creation of an endpoint for communication with <sys/socket.h> socket() function. Return value
	* stored in @c sockfd integer variable.
	*/
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (sockfd < 0){
	 error( "ERROR opening socket", 0 );
	}
	else{
		bzero((char *) &serv_addr, sizeof(serv_addr));

		serv_addr.sin_family = AF_INET;
		serv_addr.sin_addr.s_addr = INADDR_ANY;
		serv_addr.sin_port = htons( portno );

		/**
		*	@details -# If socket() succeeds, prepareSocket() binds the file descriptor returned -@c sockdf- to @p portno,
		* with any incoming address allowed. This is obtained with <sys/socket.h> bind() function.
		*/
		if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
			error( "ERROR on binding", 0 );
			close(sockfd);
			sockfd = -1;
		}
		else{

			/**
			*	@details -# If bind() succeeds, prepareSocket() set the file descriptor returned by socket() to listen
			*  with <sys/socket.h> listen() function, passing 5 as backlog argument (see @c man @c 2 @c listen).
			*  <br> Either in failure or in success, prepareSocket() prints a log message labelled as ::CONNECTION,
			*  in stderr, with printMsg() function. If listen fails, the socket pointed by @c sockfd is closed.
			*/
			if(	listen(sockfd,5) == 0 ){
				printMsg(stderr, CONNECTION, "Listening on socket %d\n", sockfd);
			}
			else{
				printMsg(stderr, CONNECTION, "Error while listening on socket %d: %s\n", sockfd, strerror(errno));
				close(sockfd);
				sockfd = -1;
			}
		}
	}

	/**
	*	@details -# Returning of sockfd value. To check the function success, it is mandatory to see wether this value
	* is greater than zero.
	*/
	return sockfd;
}


/**
 * @details
 * connectToSocket() receives a listening socket, usually opened with prepareSocket() -although it can be
 * set manually-, and blocks until a new connection is present. When a new connection is accepted or an error
 * is encountered, connectToSocket() returns either the new socket file descriptor or a negative value.
 *
 * This function has to be preceded by a successful opening of a listening socket.
 *
 * The following piece of code is self-explanatory:

 * @code
	int portno = 51717;
	int LISTEN_SOCKFD, SOCKFD;

	LISTEN_SOCKFD = prepareSocket(portno);

	if (LISTEN_SOCKFD < 0){
	 error( "ERROR opening listening socket", 0 );
	}
	else{
		SOCKFD = connectToSocket(LISTEN_SOCKFD);

		if (SOCKFD < 0){
			error( "ERROR accepting connection", 0 );
		}
		else{
			//CODE TO BE EXECUTED IN SUCCESS.
		}
	}
 * @endcode

 * <b> General behaviour </b> @n
 * The steps performed by connectToSocket() are the following:
 */
int connectToSocket(int sockfd){
	int newsockfd, clilen;
	struct sockaddr_in cli_addr;

	clilen = sizeof(cli_addr);

	/**
	*	@details -# Call to <sys/socket.h> accept() function with @p sockfd as the first argument. This blocks
	* until a new connection is accepted or an error is encountered. The return value is stored in @c newsockfd,
	* an integer variable.
	*/
	newsockfd = accept( sockfd, (struct sockaddr *) &cli_addr, (socklen_t*) &clilen);

	/**
	*	@details -# Either in failure or in success, connectToSocket() prints a log message in stderr, labelled
	* as ::CONNECTION, using printMsg() function.
	*/
	if(newsockfd > 0){
		printMsg(stderr, CONNECTION, "New socket opened: %d\n", newsockfd);
	}
	else{
		printMsg(stderr, CONNECTION, "%sERROR accepting socket: %s%s\n", KRED, strerror(errno), KRES);
	}

	/**
	*	@details -# Returning of newsockfd value. To check the function success, it is mandatory to see wether this value
	* is greater than zero.
	*/
	return newsockfd;
}


/**
 * @details
 * getCommand() reads a single byte from the socket pointed by the file descriptor @p sockfd 
 * and returns it as a command. Since it is no more than a wrapper of getData(), its processing
 * is simple.

 * See below an example of code, adapted from main.c control_connection()

 * @code
 	//Open a listening socket
 	LISTEN_COMMANDS = prepareSocket(PORT_COMMANDS);

 	//Accepts a new connection
 	SOCKET_COMMANDS = connectToSocket(LISTEN_COMMANDS);

 	//Loop to read commands from the socket
 	do{
		command = getCommand(SOCKET_COMMANDS);

		switch(command){
			//CODE TO PERFORM THE ACTIONS NEEDED BY EACH COMMAND
		}
 	}while(command != MSG_END);

 * @endcode

 * @todo Implement an error handling to check if the byte received is actually a command, in order to handle
 * the possible E-Link losses. It could be necessary to change the commands numbers in protocol.h.

 * <b> General behaviour </b> @n
 * The steps performed by getCommand() are the following:
 */
char getCommand(int sockfd){
	char command;

	/**
	*	@details
	*	-# Call of getData() with @p sockfd as the first argument and 1 as the last one, since the commands
	* size is now of 1 byte.
	*		-# If getData() returns zero -which notes an error-, the command returned is set to ::MSG_PASS.
	*		-# If getData() returns a greater value, a log message is printed in stderr, labelled as
	*		::CONNECTION, using printMsg() function.
	*	-# Returning of the command received (or ::MSG_PASS if there was any error).
	*/
	if(!getData(sockfd, &command, 1))
		command = MSG_PASS;

	if(command)
		printMsg(stderr, CONNECTION, "Command received: %d\n", command);

	return command;
}


/**
 * @details
 * getData() acts basically as a wrapper of recv(), to provide an easy call in which
 * all the server needs are fulfilled.
 *
 * Two important issues have to be considered:
 *	-# getData() iterate through a loop until any of these conditions occurs:
 * 		-# All @p n_bytes are received.
 * 		-# A fatal error is detected.
 *	-# getData() call recv() function with the following flags:
 *		-# MSG_NOSIGNAL: To avoid the automatic signal handling by recv().
 *		-# MSG_DONTWAIT: To set recv() as non-blocking function.
 *
 * getData() is used in the same way you would use recv(), taking into account the
 * already implemented loop. See below an example of code:
 *
 * @code
 	int command, value;
	command = getCommand(SOCKET_COMMANDS);

	switch(command){
		case MSG_SET_BRIGHTNESS:
			if( getData(SOCKET_COMMANDS, &value, sizeof(value)) )
				change_parameter(V4L2_CID_BRIGHTNESS, value);
		break;

		//REST OF COMMAND OPTIONS
	}
 * @endcode

 * @todo Review error handling, in the definition and in any other place where this function is used.

 * <b> General behaviour </b> @n
 * The steps performed by getData() are the following:
 */
int getData(int sockfd, void* ptr, int n_bytes){
	int n, success, bytes_sent;
	char error_string[75];

	success = 1;
	bytes_sent = 0;

	/**
	*	@details
	*	-# Start of recv() loop. It shall finish when the number of bytes actually received equals @p n_bytes.
	*/
	while (bytes_sent < n_bytes) {
		/**
		*	@details
		*	-# Call of recv(), with MSG_NOSIGNAL | MSG_DONTWAIT flags.
		*		-# If the value returned by recv is lower than zero, the loop is finished and a log message
		*		is printed in stderr, labelled as ::CONNECTION, using printMsg() function.
		*		-# If this error is different than EAGAIN (error that is throwed usually when the socket is full
				or there is a temporary failure), the connection is shut down setting ::CONNECTED variable to zero.
				If this occurs, another message noticing the fatal issue is printed.
		*/
		if ( ( n = recv(sockfd, ptr + bytes_sent, n_bytes - bytes_sent, MSG_NOSIGNAL | MSG_DONTWAIT) ) < 0 ){
			int err_num = errno;
			strerror_r(err_num, error_string, 75);

			printMsg(stderr, CONNECTION, "ERROR reading socket: %s.\n", error_string);
			success = 0;

			if(err_num != EAGAIN){
				CONNECTED = 0;
				printMsg(stderr, CONNECTION, "%sDISCONNECTING.%s\n", KRED, KRES);
			}

			break;
		}
		else{
			bytes_sent += n;
		}
	}

	/**
	*	@details
	*	-# If the loop finished succesfully, a success message is printed in stderr.
	*/
	if(success)
		printMsg(stderr, CONNECTION, "%d bytes read\n", bytes_sent);


	/**
	*	@details
	*	-# Returning of 1 or 0, depending if the function finished succesfully or any error was encountered.
	*/
	return success;
}


/**
 * @details
 * senData() acts basically as a wrapper of send(), to provide an easy call in which
 * all the server needs are fulfilled.
 *
 * Two important issues have to be considered:
 *	-# sendData() iterate through a loop until any of these conditions occurs:
 * 		-# All @p n_bytes are sent.
 * 		-# A fatal error is detected.
 *	-# sendData() call send() function with the following flags:
 *		-# MSG_NOSIGNAL: To avoid the automatic signal handling by send().
 *		-# MSG_DONTWAIT: To set send() as non-blocking function.
 *
 * sendData() is used in the same way you would use send(), taking into account the
 * already implemented loop. See below an example of code, adapted from sendImage():
 *
 * @code
	uint8_t* image_stream = NULL;
	int success;

	image_stream = malloc(sizeof(*image_stream) * 1280*960);

	memcpy(image_stream, current_frame, 1280*960);
	
	success = sendData(sockfd, image_stream, 1228800);

	free(image_stream);

	return success;
 * @endcode

 * @todo Review error handling, in the definition and in any other place where this function is used.

 * <b> General behaviour </b> @n
 * The steps performed by sendData() are the following:
 */
int sendData(int sockfd, void* ptr, int n_bytes){
	int n, success, bytes_sent;
	char error_string[75];

	success = 1;
	bytes_sent = 0;

	printMsg(stderr, CONNECTION, "Start sending: %d bytes to be sent.\n", n_bytes);

	/**
	*	@details
	*	-# Test wether the connection is correct, checking ::CONNECTED variable.
	*/
	if(CONNECTED){
		/**
		*	@details
		*	-# Start of send() loop. It shall finish when the number of bytes actually sent equals @p n_bytes.
		*/
		while (bytes_sent < n_bytes) {
			/**
			*	@details
			*	-# Call of send(), with MSG_NOSIGNAL | MSG_DONTWAIT flags.
			*		-# If the value returned by recv is lower than zero, a log message
			*		is printed in stderr, labelled as ::CONNECTION, using printMsg() function.
			*		-# If this error is different than EAGAIN (error that is throwed usually when the socket is full
					or there is a temporary failure), the loop is finished and the connection is shut down setting ::CONNECTED variable to zero.
					If this occurs, another message noticing the fatal issue is printed.
					-# If the number of bytes sent is 0, the function returns a failure but do not disconnect the server.
			*/
			if ( ( n = send(sockfd, ptr + bytes_sent, n_bytes - bytes_sent, MSG_NOSIGNAL | MSG_DONTWAIT) ) < 0 ){
				int err_num = errno;

				strerror_r(err_num, error_string, 75);
				printMsg(stderr, CONNECTION, "%sERROR writing to socket: %s.%s\n", KRED, error_string, KRES);

				//ERROR HANDLING: Socket not connected.
				//SOLUTION: Disconnect, return failure and try to reconnect.
				if(err_num != EAGAIN){
					printMsg(stderr, CONNECTION, "%sDISCONNECTING%s\n", KRED, KRES);
					CONNECTED = 0;
					success = 0;
					break;
				}

				//ERROR HANDLING: Socket temporary not available (out of space) and no bytes sent.
				//SOLUTION: Return failure.
				if(bytes_sent == 0){
					success = 0;
					break;
				}

				//ERROR HANDLING: Socket temporary not available (out of space) and some bytes sent.
				//SOLUTION: Continue the loop until all bytes have been sent <==> Do nothing.
			}
			else{
				bytes_sent += n;
				printMsg(stderr, CONNECTION, "%d of %d bytes sent.\n", bytes_sent, n_bytes);
			}
		}
	}
	else
		success = 0;

	/**
	*	@details
	*	-# A log message is printed in stderr with a summary of the function performance.
	*/
	printMsg(stderr, CONNECTION, "Finish sending: %d of %d bytes sent.\n", bytes_sent, n_bytes);

	/**
	*	@details
	*	-# Returning of 1 or 0, depending if the function finished succesfully or any error was encountered.
	*/
	return success;
}


/**
 * @details
 * sendImage() gets the latest image taken by the camera and sends it through @p sockfd, which shall be
 * a correct socket file descriptor. Since it is no more than a wrapper of sendData(), its processing
 * is simple. The importance of this function falls on how it deals with the camera lock (::camera_rw_lock)
 * to send the latest image taken.

 * The function performance is clear, and shall be called when an image should be sent.
 * This program uses this function, always, with the following call:

 @code sendImage(SOCKET_BIG); @endcode

 * The function is designed to send only new images, making use of global variables ::new_frame_send.
 * If it detects that the user wants an old image to be sent, the function will return with a failure

 * <b> General behaviour </b> @n
 * The steps performed by sendImage() are the following:
 */
int sendImage(int sockfd){
	static int COUNT = 0;
	int send_new_image = 0;
	uint8_t* image_stream = NULL;

	int success = 0;


	/**
	*	@details
	*	-# The function blocks until it can access the camera buffer, where the latest image
	*	taken is stored. This is done using the camera lock: ::camera_rw_lock.
	*/
	pthread_rwlock_rdlock( &camera_rw_lock );

		/**
		*	@details
		*	-# The function checks whether the image stored in the buffer has been sent before.
		*	This is done checking global variable ::new_frame_send, only changed from here and
		*	from DMK41BU02.c process_image() function.
		*		-# If there is no new image to be sent, the function does nothing and returns a failure.
		*		-# If there is a new image to be sent, the function does the following:
		*			-# It allocates 1280*960 bytes of memory.
		*			-# It copies the shared buffer into the memory allocated.
		*			-# It sets ::new_frame_send to zero, to avoid future calls of sendImage() to send the same image.

		*/
		if(new_frame_send){
			image_stream = malloc(IMG_FILE_SIZE);

			memcpy(image_stream, current_frame, IMG_FILE_SIZE);

			new_frame_send = 0;
			send_new_image = 1;
		}
		else
			send_new_image = 0;

	/**
	*	@details
	*	-# The function unlocks ::camera_rw_lock.
	*/
	pthread_rwlock_unlock( &camera_rw_lock );

	/**
	*	@details
	*	-# If a new image has to be sent, sendImage() calls sendData() the proper way and stores its
	*	return value to return to the caller of sendImage().
	*/
	if(send_new_image){
		success = sendData(sockfd, image_stream, IMG_FILE_SIZE);
	}

	/**
	*	@details
	*	-# The memory is freed, even if it was not allocated.This throws no error because the pointer
	*	is initialised to NULL.
	*/
	free(image_stream);

	/**
	*	@details
	*	-# Returns the return value of sendData() or 0 if no image was sent.
	*/
	return success;
}


/**
 * @details
 * sendAccAndMag() gets the latest measurements taken by LSM303 magnetometer/accelerometer,
 * serializes them and sends them through @p sockfd, which should be a correct socket file
 * descriptor.
 * Since it is no more than a wrapper of sendData(), its processing is simple. The importance
 * of this function falls on how it deals with the LSM303 files lock (::magnetometer_rw_lock
 * and ::accelerometer_rw_lock).

 * The function performance is clear, and shall be called when the measurements should be sent.
 * This program uses this function, always, with the following call:

   @code sendAccAndMag(read_mag, read_acc, SOCKET_SMALL); @endcode

 * where read_mag and read_acc are the file descriptors associated to the LSM303 files, in which
 * all the measurements are stored. These file descriptors should be opened only for reading.

 * @todo Implement an algorithm, like in sendImage(), to avoids the sending of the same measurements
 * more than once.

 * <b> General behaviour </b> @n
 * The steps performed by sendAccAndMag() are the following:
 */
int sendAccAndMag(FILE* mag_file, FILE* acc_file, int sockfd){
	#define X   0
	#define Y   1
	#define Z   2
	#define A_GAIN 0.004    	//[G/LSB] FS=10
	#define M_XY_GAIN 1100   	//[LSB/Gauss] GN=001
	#define M_Z_GAIN 980	//[LSB/Gauss] GN=001
	#define T_GAIN 8	//[LSB/ºC]

	uint8_t buffer[MAG_FM_SIZE + ACC_FM_SIZE];
	int success = 0;

	/**
	*	@details
	*	-# The function blocks until it can access the magnetometer file, where the
	*	magnetometer measurements are written. This lock is ::magnetometer_rw_lock.
	*		-# When the file can be accessed, it moves the position indicator to the
	*		latest measurement. Then, the latest magnetometer measurement is stored in
	*		the first 6 bytes of a buffer.
	*	-# The function unlocks ::magnetometer_rw_lock.
	*/
	pthread_rwlock_rdlock( &magnetometer_rw_lock );
		fseek(mag_file, -MAG_FM_SIZE, SEEK_END);
		fread(buffer, sizeof(*buffer), MAG_FM_SIZE, mag_file);
	pthread_rwlock_unlock( &magnetometer_rw_lock );

	/**
	*	@details
	*	-# The function blocks until it can access the accelerometer file, where the
	*	accelerometer measurements are written. This lock is ::accelerometer_rw_lock.
	*		-# When the file can be accessed, it moves the position indicator to the
	*		latest measurement. Then, the latest accelerometer measurement is stored in
	*		the last 6 bytes of the buffer.
	*	-# The function unlocks ::accelerometer_rw_lock.
	*/
	pthread_rwlock_rdlock( &accelerometer_rw_lock );
		fseek(acc_file, -ACC_FM_SIZE, SEEK_END);
		fread(buffer+MAG_FM_SIZE, sizeof(*buffer), ACC_FM_SIZE, acc_file);
	pthread_rwlock_unlock( &accelerometer_rw_lock );

	/**
	*	@details
	*	-# Both measurements are processed in order to print a human readable message. A log message
	*	is then printed with printMsg() function, labelled as ::CONNECTION.
	*/
	int16_t m[3];
	float MAG[3];

	*m = (int16_t)(buffer[1] | buffer[0] << 8);
	*(m+1) = (int16_t)(buffer[5] | buffer[4] << 8);
	*(m+2) = (int16_t)(buffer[3] | buffer[2] << 8);

	*(MAG+0) = (float) *(m+0)/M_XY_GAIN;
	*(MAG+1) = (float) *(m+1)/M_XY_GAIN;
	*(MAG+2) = (float) *(m+2)/M_Z_GAIN;

	int16_t a[3];
	float accF[3];

	*a = (int16_t)(buffer[0+MAG_FM_SIZE] | buffer[MAG_FM_SIZE+1] << 8) >> 4;
    *(a+1) = (int16_t)(buffer[MAG_FM_SIZE+2] | buffer[MAG_FM_SIZE+3] << 8) >> 4;
    *(a+2) = (int16_t)(buffer[MAG_FM_SIZE+4] | buffer[MAG_FM_SIZE+5] << 8) >> 4;

	*(accF+0) = (float) *(a+0)*A_GAIN;
	*(accF+1) = (float) *(a+1)*A_GAIN;
	*(accF+2) = (float) *(a+2)*A_GAIN;

	printMsg(stderr, CONNECTION, "Sending magnetometer: %4.3f %4.3f %4.3f\n", MAG[0],MAG[1],MAG[2]);
	printMsg(stderr, CONNECTION, "Sending accelerometer: %4.3f %4.3f %4.3f\n", accF[0],accF[1],accF[2]);

	/**
	*	@details
	*	-# The buffer in which both measurements were stored is sent to @p sockfd using sendData() function.
	*/
	success = sendData(sockfd, buffer, MAG_FM_SIZE + ACC_FM_SIZE);
	
	/**
	*	@details
	*	-# If sendData() was succesfull, a success log message is printed. If it returned a failure, an error
	*	message is also printed.
	*/
	if(success)
		printMsg(stderr, CONNECTION, "Sent new buffer.\n");
	else
		printMsg(stderr, CONNECTION, "Buffer not sent.\n");

	/**
	*	@details
	*	-# The return value of sendData() is returned.
	*/
	return success;
}