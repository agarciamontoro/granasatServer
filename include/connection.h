#ifndef CONNECTION_H__
#define CONNECTION_H__

#define PORT_COMMANDS	 51717
#define PORT_BIG_DATA	 51718
#define PORT_SMALL_DATA	 51719

#include <netinet/in.h>
#include <errno.h>

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

#endif