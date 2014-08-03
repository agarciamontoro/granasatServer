#include "connection.h"

void error(char *msg) {
	perror(msg);
	exit(1);
}

int getData(int sockfd) {
	char buffer[32];
	int n;

	if ((n = read(sockfd, buffer, 31)) < 0)
		error("ERROR reading from socket");

	buffer[n] = '\0';
	printMsg(stderr, "[Connection]:\tBytes received: %d\n", n);
	return atoi(buffer);
}

char getCommand(int sockfd){
	char command;

	if (recv(sockfd, &command, 1, MSG_DONTWAIT) < 0){
		if(errno != EAGAIN)
			error("ERROR reading from socket");
		else
			command = 0;
	}

	if(command)
		printMsg(stderr, "[Connection]:\tCommand received: %d\n", command);
	
	return command;
}

int getInt(int sockfd){
	int value;

	if (recv(sockfd, &value, sizeof(int), 0) < 0)
		perror("ERROR reading from socket");


	printMsg(stderr, "[Connection]:\t\tValue received: %d\n", value);
	return value;
}

void sendData(int x, int sockfd) {
	char buffer[32];
	int n;

	// Prints to a buffer.
	sprintf( buffer, "%d\n", x );

	// Sends to the server.
	if ((n = write(sockfd, buffer, strlen(buffer))) < 0 )
		perror("ERROR writing to socket");

	buffer[n] = '\0';
}

void enableConnection(int* sockfd, int portno, int* clilen, struct sockaddr_in* cli_addr ){
	struct sockaddr_in serv_addr;

	printMsg(stderr, "[Connection]:\tUsing port #%d\n", portno);

	*sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (sockfd < 0)
		error(("ERROR opening socket"));

	bzero((char *) &serv_addr, sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);

	if (bind(*sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
		error(("ERROR on binding"));

	listen(*sockfd, 5);
	*clilen = sizeof(*cli_addr);
}

void sendData_v2(int sockfd, void* ptr, int n_bytes){
	int n;
	int bytes_sent = 0;

	while (bytes_sent < n_bytes) {
		if ((n = write(sockfd, ptr + bytes_sent, n_bytes - bytes_sent)) < 0)
			error("ERROR writing to socket");
		else{
			bytes_sent += n;
			printMsg(stderr, "[Connection]:\t%d bytes sent\n", bytes_sent);
		}
	}
}

void sendImage(int sockfd, uint8_t* image_stream){
	int send_new_image = 0;

	pthread_rwlock_rdlock( &camera_rw_lock );

		if(new_frame_send){
			memcpy(image_stream, current_frame, 1280*960);
			new_frame_send = 0;
			send_new_image = 1;
		}
		else
			send_new_image = 0;

	pthread_rwlock_unlock( &camera_rw_lock );

	if(send_new_image)
		sendData_v2(sockfd, image_stream, 1280*960);
}

void sendAcc(int sockfd){

}

void sendMag(int sockfd){

}

void sendAccAndMag(int sockfd){
	uint8_t buffer[12];
	static int count = 0;

	count++;

	srand(time(NULL));

	int i;
	for (i = 0; i < 12; ++i){
		buffer[i] = count*i;//(uint8_t)rand()%100;
	}
	sendData_v2(sockfd, buffer, sizeof(*buffer) * 12);

	printMsg(stderr, "[Connection]:\tSent new buffer.\n");

}

void createCommChannel(struct communication* comm){
	int clilen;
	struct sockaddr_in cli_addr;

	enableConnection( &(comm->sockfd), comm->portno, &clilen, &cli_addr );

	printMsg(stderr, "[Connection]:\tWaiting for client to open communication channel. Socket: %d.\n", comm->sockfd);

	if ( (comm->new_sockfd = accept(comm->sockfd, (struct sockaddr *) &cli_addr, (socklen_t*) &clilen)) < 0);
		error(("ERROR on accept"));

	printMsg(stderr, "[Connection]:\tOpened new communication channel with client.\n");
}

int openSocket(int portno){
	int sockfd, newsockfd, clilen;
	struct sockaddr_in serv_addr, cli_addr;
	int n;
	int data;

	printMsg(stderr, "[Connection]:\tUsing port #%d\n", portno );

	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (sockfd < 0) 
	 error( "ERROR opening socket" );

	bzero((char *) &serv_addr, sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons( portno );

	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
		error( "ERROR on binding" );

	listen(sockfd,5);
	clilen = sizeof(cli_addr);

	newsockfd = accept( sockfd, (struct sockaddr *) &cli_addr, (socklen_t*) &clilen);
	
	if(newsockfd < 0)
	    error( "ERROR on accept" );

	return newsockfd;
}

int prepareSocket(int portno){
	int sockfd, newsockfd;
	struct sockaddr_in serv_addr;
	int n;
	int data;

	printMsg(stderr, "[Connection]:\tUsing port #%d\n", portno );

	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (sockfd < 0) 
	 error( "ERROR opening socket" );

	bzero((char *) &serv_addr, sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons( portno );

	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
		error( "ERROR on binding" );

	listen(sockfd,5);

	return sockfd;
}

int connectToSocket(int sockfd){
	int newsockfd, clilen;
	struct sockaddr_in cli_addr;

	clilen = sizeof(cli_addr);

	newsockfd = accept( sockfd, (struct sockaddr *) &cli_addr, (socklen_t*) &clilen);
	return newsockfd;
}