#ifndef CONNECTION_H__
#define CONNECTION_H__

#include <netinet/in.h>
#include <errno.h>

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

void sendImage(int sockfd, uint8_t* image_stream);

void sendAcc(int sockfd);

void sendMag(int sockfd);

void sendAccAndMag(int sockfd);

void createCommChannel(struct communication* comm);

int openSocket(int portno);


#endif