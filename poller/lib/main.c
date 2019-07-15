// Client side implementation of UDP client-server model
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
//#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "io_ring.h"

#define PORT	8080
#define MAXLINE 1024
#define SERVER_ADDR (10<<24|1<<16|4<<8|38) /*10.1.4.38*/

#define max(a,b) (a < b) ? b : a
// Driver code
int main(void)
{
	int sockfd;
	int i, len;
	int flags = MSG_CONFIRM;
	char buffer[MAXLINE];
	char *hello = "Hello from client";
	struct sockaddr_in	 servaddr;

	// Creating socket file descriptor
	if ( (sockfd = ir_socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
		perror("socket creation failed");
		exit(EXIT_FAILURE);
	}

	memset(&servaddr, 0, sizeof(servaddr));

	// Filling server information
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(PORT);
	servaddr.sin_addr.s_addr = htonl(SERVER_ADDR);

	for (i = 0; i < (1<<25); i++) {
		ir_sendto(sockfd, (const char *)hello, max(strlen(hello), 42),
			flags, (const struct sockaddr *) &servaddr,
				sizeof(servaddr));
	}
	printf("Hello message sent.(%d)\n", i);
/*
	n = recvfrom(sockfd, (char *)buffer, MAXLINE,
				MSG_WAITALL, (struct sockaddr *) &servaddr,
				&len);
	buffer[n] = '\0';
	printf("Server : %s\n", buffer);
*/
	//close(sockfd);
	return 0;
}

