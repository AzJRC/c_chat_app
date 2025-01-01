/* sock_client.c */
#include "sock_util.h"

#define SRV_ADDR "127.0.0.1"
#define SRV_PORT 2357


int main() {
		
	int sfd = createTCPv4Socket();	
	struct sockaddr_in *addr = createV4Sock(SRV_ADDR, SRV_PORT);	

	// connect to the address using the socket.
	if (connect(sfd, (struct sockaddr *) addr, sizeof(*addr)) == -1) {
		printf("- something went wrong: %s", strerror(errno));
		return -1; 
	}

	printf("+ connection stablished.\n");

	// beginning of chat application

	char *line = NULL;
	size_t line_s = 0;	
	ssize_t char_count;

	while (true) {

		if ((char_count = getline(&line, &line_s, stdin)) < 0) {
			return -1;
		}

		// finish conversation.
		if (strlen(line) == 1) {
			send(sfd, "//end", 5, 0);
			break;
		};  
		
		
		if (send(sfd, line, char_count, 0) == -1) {
			return -1;
		}
		//	char buff_recv[1024];
		//	if (recv(sfd, buff_recv, 1024, 0) == -1) return -1;
		//	printf("+ message received: %s\n", buff_recv);
	}	
	
	close(sfd);
	return 0;
}
