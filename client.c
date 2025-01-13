/* sock_client.c */
#include "util.h"

#define SRV_ADDR "127.0.0.1"
#define SRV_PORT 2357

#define NEWCONN_CMD "//newconn\n"
#define ENDCONN_CMD "//endconn\n"


int main() {
		
	int sfd = createTCPv4Socket();	
	struct sockaddr_in *addr = createSockaddrStruct(SRV_ADDR, SRV_PORT);	

	// connect to the address using the socket.
	if (connect(sfd, (struct sockaddr *) addr, sizeof(*addr)) == -1) {
		printf("- something went wrong: %s", strerror(errno));
		return -1; 
	}


	//start clientRecvThread() to receive messages assycronously
	runThreadRecv(sfd);

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
			send(sfd, ENDCONN_CMD, strlen(ENDCONN_CMD), 0);
			break;
		};  
		
		
		if (send(sfd, line, char_count, 0) == -1) {
			return -1;
		}
	}	
	
	close(sfd);
	return 0;
}


void runThreadRecv(int sfd) {
	int *sfd_ptr = malloc(sizeof(int));  // Allocate memory
    if (!sfd_ptr) {
        LOG_ERROR_MESSAGE("Error with malloc(): %s.\n", strerror(errno));
        return;
    }

    *sfd_ptr = sfd;  // Store the value of sfd in allocated memory
	pthread_t id;
	pthread_create(&id, NULL, threadRecv, sfd_ptr);
}


void *threadRecv(void *sfd_arg) {

	int sfd = *(int *)sfd_arg;
	free(sfd_arg);

	char buffer[1024];
	while(true) {
		
		ssize_t char_count = recv(sfd, buffer, sizeof(buffer)-1, 0);
		if (char_count < 0) {
			LOG_ERROR_MESSAGE("Error with recv(): %s.\n", strerror(errno));
			return NULL;
		}

		if (char_count == 0) break;

		buffer[char_count] = 0;
		LOG_INFO_MESSAGE("Message received: [ %s ]\n", buffer);

	}

	close(sfd);

	return 0;
}

