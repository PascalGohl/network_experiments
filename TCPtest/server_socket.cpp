/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>

#define IMAGE_SIZE 360960 //=752*480
#define PACKAGE_SIZE 512

void error(const char *msg)
{
	perror(msg);
	exit(1);
}

int main(int argc, char *argv[])
{
	const char* data[IMAGE_SIZE]; // as one b/w image
	const struct timespec wait_time = {0, 30000000}; //time between measurements

	int sockfd, newsockfd, portno;
	socklen_t clilen;
	char buffer[256];
	struct sockaddr_in serv_addr, cli_addr;
	int n;
	if (argc < 2) {
		fprintf(stderr,"ERROR, no port provided\n");
		exit(1);
	}
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		error("ERROR opening socket");
	bzero((char *) &serv_addr, sizeof(serv_addr));
	portno = atoi(argv[1]);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	if (bind(sockfd, (struct sockaddr *) &serv_addr,
			sizeof(serv_addr)) < 0)
		error("ERROR on binding");
	listen(sockfd,5);
	clilen = sizeof(cli_addr);
	newsockfd = accept(sockfd,
			(struct sockaddr *) &cli_addr,
			&clilen);
	if (newsockfd < 0)
		error("ERROR on accept");
	bzero(buffer,256);


	*((unsigned int *) data+0) = 0; //udp package counter at the beginning of the data
	*((unsigned int *) data+4) = 0; //sensor package counter

	while (1)
	{
		*((unsigned int *) data+4) += 1;
		// send sensor package over UDP
		int i = 0;
		for(; i < IMAGE_SIZE/PACKAGE_SIZE; i++)
		{
			// increment the counter
			*((unsigned int *) data+0) += 1;
			n = send(newsockfd,data,PACKAGE_SIZE,0);
			if (n < 0)
				error("ERROR writing to socket");
		}
//
//		// send second image
//		i = 0;
//		for(; i < IMAGE_SIZE/PACKAGE_SIZE; i++)
//		{
//			// increment the counter
//			*((unsigned int *) data+0) += 1;
//			n = write(newsockfd,data,PACKAGE_SIZE);
//			if (n < 0)
//				error("ERROR writing to socket");
//		}

		nanosleep(&wait_time, NULL); //wait 3ms
	}

	close(newsockfd);
	close(sockfd);
	return 0;
}
