#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "boost/date_time/posix_time/posix_time.hpp"

#define IMAGE_SIZE 360960 //=752*480
#define PACKAGE_SIZE IMAGE_SIZE

void error(const char *msg)
{
	perror(msg);
	exit(0);
}

int main(int argc, char *argv[])
{
	int sockfd, portno, n;
	struct sockaddr_in serv_addr;
	struct hostent *server;

	char mesg[PACKAGE_SIZE];
	if (argc < 3) {
		fprintf(stderr,"usage %s hostname port\n", argv[0]);
		exit(0);
	}
	portno = atoi(argv[2]);
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		error("ERROR opening socket");
	server = gethostbyname(argv[1]);
	if (server == NULL) {
		fprintf(stderr,"ERROR, no such host\n");
		exit(0);
	}
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr,
			(char *)&serv_addr.sin_addr.s_addr,
			server->h_length);
	serv_addr.sin_port = htons(portno);
	if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
		error("ERROR connecting");
	printf("Please enter the message: ");
	bzero(mesg,PACKAGE_SIZE);

	boost::posix_time::ptime time_start(boost::posix_time::microsec_clock::local_time());
	boost::posix_time::time_duration interval(0,0,1,0);

	unsigned int start_package_id = 0;
	unsigned int last_package_id = 0;
	int packages_lost = 0;

	unsigned int package_counter[32] = {0};

	for (;;)
	{
		n = read(sockfd,mesg,PACKAGE_SIZE);
		if (n < 0)
			error("ERROR reading from socket");
		mesg[n] = 0;

		unsigned int package_id = *((unsigned int *) mesg);
		unsigned int message_id = *((unsigned int *) mesg + 4);

		//		if(last_package_id + 1 != package_id)
		//		{
		////			printf("%d packages missed.\n", *((int *) mesg) - last_package_id +1);
		//			packages_lost++;
		//		}
		//

		package_counter[message_id%32] +=1;

		last_package_id = package_id;

		boost::posix_time::ptime time_end(boost::posix_time::microsec_clock::local_time());
		boost::posix_time::time_duration duration(time_end - time_start);

		// calculate the link bandwidth every second
		if(duration > interval)
		{
			printf("received %d packages/sec ",*((int *) mesg) - start_package_id);
			printf("=> %f MBit/s ", double(*((int *) mesg) - start_package_id)*PACKAGE_SIZE*8 / 1000000);

			for(int j = message_id-30; j<message_id-2; j++)
			{
				packages_lost += IMAGE_SIZE/PACKAGE_SIZE - package_counter[j%32];
				printf("counter: %d\n",package_counter[j%32]);
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

	printf("%s\n",mesg);
	close(sockfd);
	return 0;
}
