/* sock_server.c */
#include "sock_util.h"

#define SRV_ADDR ""
#define SRV_PORT 2357
#define LST_BACKLOG 2

int processConn(int sfd_client) {
	/*
	 Process a connection by listening to a received accepted connection from `acceptedConn()` using
	 the `listen()^ function.
	 Loop indifinitely until the connection is closed by the client or an error arise.
	*/

	char buff_recv[1024];
	ssize_t recv_content;
	while (true) {
		if ( (recv_content = recv(sfd_client, buff_recv, 1024, 0)) < 0) {
			printf("- error with recv(): %s", strerror(errno));
			return -1;
		}

		if (recv_content == 1 || recv_content == 0) break;  // stop if user sends nothing or closes the connection.

		buff_recv[recv_content] = 0;
		printf("+ message received: %s\n", buff_recv);
	}

	return 0;
} 

int main() {
	// create socket	
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

	// accept incoming connections
	struct acceptedConn *client_conn = acceptNewConn(sfd);
	int sfd_client = client_conn->sfd_client;
		
	// receive and process connections
	processConn(sfd_client);	


	printf("+ closing sockets...\n");

	close(sfd_client);
	shutdown(sfd, SHUT_RDWR);
	
	printf("+ program end.\n");

	return 0;
}
