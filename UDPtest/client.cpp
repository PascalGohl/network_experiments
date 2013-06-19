#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <strings.h>
#include <ctime>
#include "boost/date_time/posix_time/posix_time.hpp"

#define IMAGE_SIZE 360960 //=752*480
#define PACKAGE_SIZE 8000

int main(int argc, char**argv)
{
	int sockfd,n;
	struct sockaddr_in servaddr,cliaddr;
	socklen_t len;
	char mesg[PACKAGE_SIZE];

	// create socket
	sockfd=socket(AF_INET,SOCK_DGRAM,0);

	// configure socket
	bzero(&servaddr,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
	servaddr.sin_port=htons(32000);
	bind(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr));

	int socket_size = 1024 * 1024000;
	if (setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &socket_size, sizeof(socket_size)) == -1)
	{
		printf("Increasing socket size failed.");
	}

	boost::posix_time::ptime time_start(boost::posix_time::microsec_clock::local_time());
	boost::posix_time::time_duration interval(0,0,1,0);

	unsigned int start_package_id = 0;
	unsigned int last_package_id = 0;
	int packages_lost = 0;

	unsigned int package_counter[32] = {0};

	for (;;)
	{
		n = recv(sockfd,mesg,PACKAGE_SIZE,0);
		mesg[n] = 0;
		unsigned int package_id = *((unsigned int *) mesg);
		unsigned int message_id = *((unsigned int *) mesg + 4);
//		if(last_package_id + 1 != *((int *) mesg))
//		{
//			printf("%d packages missed.\n", *((int *) mesg) - last_package_id +1);
//			packages_lost++;
//		}
		last_package_id = *((int *) mesg);
		package_counter[message_id%32] +=1;

		boost::posix_time::ptime time_end(boost::posix_time::microsec_clock::local_time());
		boost::posix_time::time_duration duration(time_end - time_start);

		// calculate the link bandwith every second
		if(duration > interval)
		{
			printf("received %d packages/sec ",*((int *) mesg) - start_package_id);
			printf("=> %f MBit/s ", double(*((int *) mesg) - start_package_id)*PACKAGE_SIZE*8 / 1000000);
			for(int j = message_id-30; j<message_id-2; j++)
			{
				packages_lost += IMAGE_SIZE/PACKAGE_SIZE - package_counter[j%32];
				printf("package counter: %d\n", package_counter[j%32]);
			}
			printf("%d packages lost.\n", packages_lost);
			time_start = boost::posix_time::microsec_clock::local_time();
			start_package_id = *((int *) mesg);
			packages_lost = 0;
			for(int j = 0; j<32; j++)
			{
				package_counter[j] = 0;
			}
		}
	}
}

