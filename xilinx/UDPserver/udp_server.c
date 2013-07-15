#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <strings.h>
#include <arpa/inet.h>
#include <time.h>

#define IMAGE_SIZE 360960 //=752*480
#define PACKAGE_SIZE 8000

int sockfd;
struct sockaddr_in servaddr;
const struct timespec wait_time = {0, 50000000}; //time between measurements

int main(int argc, char**argv)
{
	const char* data[IMAGE_SIZE]; // as one b/w image

	if (argc != 2)
	{
		printf("usage:  server <client IP address>\n");
		return(1);
	}

	sockfd=socket(AF_INET,SOCK_DGRAM,0);
	bzero(&servaddr,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr=inet_addr(argv[1]);
	servaddr.sin_port=htons(32000);

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
			sendto(sockfd, data, PACKAGE_SIZE,0,
					(struct sockaddr *)&servaddr,sizeof(servaddr));
		}
		i = 0;
		for(; i < IMAGE_SIZE/PACKAGE_SIZE; i++)
		{
			// increment the counter
			*((unsigned int *) data+0) += 1;
			sendto(sockfd, data, PACKAGE_SIZE,0,
					(struct sockaddr *)&servaddr,sizeof(servaddr));
		}

		nanosleep(&wait_time, NULL); //wait 30ms
	}
}
