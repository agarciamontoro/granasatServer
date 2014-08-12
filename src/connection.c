#include "connection.h"

int CONNECTED = 0;

int SOCKET_COMMANDS = 0;
int SOCKET_BIG = 0;
int SOCKET_SMALL = 0;

int LISTEN_COMMANDS = 0;
int LISTEN_BIG = 0;
int LISTEN_SMALL = 0;

void error(const char *msg, int status) {
	char error_string[75];

	strerror_r(errno, error_string, 75);

	printMsg(stderr, CONNECTION, "%s%s: %s.\n", KLRE, msg, error_string);
	
	if(status){
		printMsg(stderr, CONNECTION, "DISCONNECTING.\n");
		CONNECTED = 0;
	}
}

int prepareSocket(int portno){
	int sockfd, newsockfd;
	struct sockaddr_in serv_addr;
	int n;
	int data;

	printMsg(stderr, CONNECTION, "Using port #%d\n", portno );

	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (sockfd < 0) 
	 error( "ERROR opening socket", 0 );

	bzero((char *) &serv_addr, sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons( portno );

	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
		error( "ERROR on binding", 0 );

	if(	listen(sockfd,5) == 0 ){
		printMsg(stderr, CONNECTION, "Listening on socket %d\n", sockfd);
	}
	else{
		printMsg(stderr, CONNECTION, "Error while listening on socket %d: %s\n", sockfd, strerror(errno));
	}

	return sockfd;
}

int connectToSocket(int sockfd){
	int newsockfd, clilen;
	struct sockaddr_in cli_addr;

	clilen = sizeof(cli_addr);

	newsockfd = accept( sockfd, (struct sockaddr *) &cli_addr, (socklen_t*) &clilen);

	if(newsockfd > 0)
		printMsg(stderr, CONNECTION, "New socket opened: %d\n", newsockfd);
	else
		printMsg(stderr, CONNECTION, "%sERROR accepting socket: %s%s\n", KRED, strerror(errno), KRES);

	return newsockfd;
}

char getCommand(int sockfd){
	char command;

	if(!getData(sockfd, &command, 1))
		command = 0;

	if(command)
		printMsg(stderr, CONNECTION, "Command received: %d\n", command);

	return command;
}

int getData(int sockfd, void* ptr, int n_bytes){
	int n, success, bytes_sent;
	char error_string[75];

	success = 1;
	bytes_sent = 0;

	while (bytes_sent < n_bytes) {
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

	if(success)
		printMsg(stderr, CONNECTION, "%d bytes read\n", bytes_sent);

	return success;
}

int sendData(int sockfd, void* ptr, int n_bytes){
	int n, success, bytes_sent;
	char error_string[75];

	success = 1;
	bytes_sent = 0;

	printMsg(stderr, CONNECTION, "Start sending: %d bytes to be sent.\n", n_bytes);

	if(CONNECTED){
		while (bytes_sent < n_bytes) {
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

	printMsg(stderr, CONNECTION, "Finish sending: %d of %d bytes sent.\n", bytes_sent, n_bytes);

	return success;
}

int sendImage(int sockfd){
	static int COUNT = 0;
	int send_new_image = 0;
	uint8_t* image_stream = NULL;

	int success = 0;

	pthread_rwlock_rdlock( &camera_rw_lock );

		if(new_frame_send){
			image_stream = malloc(sizeof(*image_stream) * 1280*960);

			memcpy(image_stream, current_frame, 1280*960);

			new_frame_send = 0;
			send_new_image = 1;
		}
		else
			send_new_image = 0;

	pthread_rwlock_unlock( &camera_rw_lock );

	if(send_new_image){
		success = sendData(sockfd, image_stream, 1228800);
	}

	free(image_stream);

	return success;
}

int sendAccAndMag(FILE* mag_file, FILE* acc_file, int sockfd){
	uint8_t buffer[12];
	int success = 0;

	fread(buffer, sizeof(uint8_t), 6, mag_file);
	fread(buffer+6, sizeof(uint8_t), 6, acc_file);

	success = sendData(sockfd, buffer, sizeof(*buffer) * 12);
	
	if(success)
		printMsg(stderr, CONNECTION, "Sent new buffer.\n");
	else
		printMsg(stderr, CONNECTION, "Buffer not sent.\n");

	return success;
}