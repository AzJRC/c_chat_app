/* sock_server.c */
#include "sock_util.h"


#define SRV_ADDR ""
#define SRV_PORT 2357
#define LST_BACKLOG 2


int main() {
	// create TCP IPv4 socket for the server	
	int sfd = createTCPv4Socket();		
	struct sockaddr_in *addr = createV4Sock(SRV_ADDR, SRV_PORT);

	// assign host's address and port to the socket so that listen() can work
	if (bind(sfd, (struct sockaddr *) addr, sizeof(*addr)) == -1) {
		printf("- error with bind(): %s.\n", strerror(errno));
		return -1;
	}
	printf("+ address bind succesfull.\n");

	// listen for incomming connections
	if (listen(sfd, LST_BACKLOG) == -1) {
		printf("- error with listen(): %s.\n", strerror(errno));
		return -1;
	}
	printf("+ listen for incomming connections.\n");	

	// accept incoming connections in a thread
	threadConnections(sfd);

	printf("+ closing sockets...\n");

	shutdown(sfd, SHUT_RDWR);
	
	printf("+ program end.\n");

	return 0;
}
