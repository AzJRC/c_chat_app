/* sock_server.c */
#include "sock_util.h"


#define SRV_ADDR ""
#define SRV_PORT 2357
#define LST_BACKLOG 2

void *threadServerConsole() {
	char *line = NULL;
	size_t line_size = 0;
	while (true) {
		printf("> ");
		ssize_t char_count = getline(&line, &line_size, stdin);
		line[char_count] = 0;
		printf("%s", line);
	}
}

int runServerConsole() {
	runInThread(threadServerConsole, NULL, 0);	
	return 0;
}

int main() {
	// create TCP IPv4 socket for the server	
	int sfd = createTCPv4Socket();		
	struct sockaddr_in *addr = createV4Sock(SRV_ADDR, SRV_PORT);

	// assign host's address and port to the socket so that listen() can work
	if (bind(sfd, (struct sockaddr *) addr, sizeof(*addr)) == -1) {
		printf("- error with bind(): %s.\n", strerror(errno));
		return -1;
	}

	// listen for incomming connections
	if (listen(sfd, LST_BACKLOG) == -1) {
		printf("- error with listen(): %s.\n", strerror(errno));
		return -1;
	}

	// start server console management
	runServerConsole();

	// accept incoming connections in a thread
	threadConnections(sfd);


	shutdown(sfd, SHUT_RDWR);
	

	return 0;
}
