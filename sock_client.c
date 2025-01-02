/* sock_client.c */
#include "sock_util.h"

#define SRV_ADDR "127.0.0.1"
#define SRV_PORT 2357

#define NEWCONN_CMD "//newconn\n"
#define ENDCONN_CMD "//endconn\n"


int requestClientID(int sfd, char *response) {
	
	if (send(sfd, NEWCONN_CMD, strlen(NEWCONN_CMD), 0) == -1) {
		return -1;
	}

	printf("+ client_id requested.\n");
	
	//receive in clientRecvThread()
	//if (recv(sfd, response, sizeof(char)*16, 0) == -1) {
	//	return -1;
	//}
	// printf("+ received client id: %s\n", response);
	
	return 0;
}


int main() {
		
	int sfd = createTCPv4Socket();	
	struct sockaddr_in *addr = createV4Sock(SRV_ADDR, SRV_PORT);	

	// connect to the address using the socket.
	if (connect(sfd, (struct sockaddr *) addr, sizeof(*addr)) == -1) {
		printf("- something went wrong: %s", strerror(errno));
		return -1; 
	}

	printf("+ connection stablished.\n");

	// request a client_id
	char client_id;
	if (requestClientID(sfd, &client_id) < 0) {
		return -1;
	}	

	// beginning of chat application
	char *line = NULL;
	size_t line_s = 0;	
	ssize_t char_count;

	//start clientRecvThread() to receive messages assycronously
	// clientRecvThread();

	while (true) {
		if ((char_count = getline(&line, &line_s, stdin)) < 0) {
			return -1;
		}

		// finish conversation.
		if (strlen(line) == 1) {
			send(sfd, ENDCONN_CMD, strlen(ENDCONN_CMD), 0);
			break;
		};  
		
		
		if (send(sfd, line, char_count, 0) == -1) {
			return -1;
		}

		// receive meesages in a different thread
		//	char buff_recv[1024];
		//	if (recv(sfd, buff_recv, 1024, 0) == -1) return -1;
		//	printf("+ message received: %s\n", buff_recv);
	}	
	
	close(sfd);
	return 0;
}
