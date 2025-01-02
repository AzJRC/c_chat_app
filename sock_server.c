/* sock_server.c */
#include "sock_util.h"

#include <stdlib.h>
#include <pthread.h>

#define SRV_ADDR ""
#define SRV_PORT 2357
#define LST_BACKLOG 2


void *threadNewConn(void *arg) {
	int sfd_client = *(int *)arg; 
	free(arg);

	listenConn(sfd_client);	
	close(sfd_client);

	return NULL;
}

int threadConnections(int sfd_srv) {

	while (true) {
		struct acceptedConn *client_conn = acceptNewConn(sfd_srv);
		if (client_conn->error < 0) {
			printf("- Erro with AcceptNewConn(): %s.\n", strerror(errno));
			return -1;
		}
		int sfd_client = client_conn->sfd_client;
			

		// Run threadNewConn() from a new thread pthread_create()
		pthread_t id;
		int *arg = malloc(sizeof(int));
		if (!arg) {
			printf("Error with malloc(): %s\n", strerror(errno));
			return -1;
		}
		*arg = sfd_client;

		if(pthread_create(&id, NULL, threadNewConn, arg) != 0) {
			printf("- Error with pthread(): %s\n", strerror(errno));
			free(arg);
			return -1;
		}
	}
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

	// accept incoming connections in a thread
	threadConnections(sfd);

	printf("+ closing sockets...\n");

	shutdown(sfd, SHUT_RDWR);
	
	printf("+ program end.\n");

	return 0;
}
