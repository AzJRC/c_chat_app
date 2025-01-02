#include "sock_util.h"


int createTCPv4Socket() {
	/*
	 Create a socket file descriptor using the socket() function.
	*/

	int sfd = socket(AF_INET, SOCK_STREAM, 0);  // create a TCP v4 socket fd.	
	if (sfd < 0) {
		printf("- error while creating a TCP IPv4 Socket: %s.\n", strerror(errno));
		return -1;
	}
	printf("+ socket created.\n");
	return sfd;
}


struct sockaddr_in *createV4Sock(char *ip, int port) { 
	/*
	 Create a IPv4 socket address using `sockaddr_in`.
	 This function allocated memory for the `sockaddr_in` structure that needs to be free
	 manually.
	*/

	struct sockaddr_in *addr = malloc(sizeof(struct sockaddr_in));  // sockaddr_in is the struct used por AF_INET (IPv4).
	addr->sin_family = AF_INET;
	addr->sin_port = htons(port);  // htons() convert integers into network byte order (Endians)
	if (strlen(ip) == 0) {
		addr->sin_addr.s_addr = INADDR_ANY;	
	} else {
		inet_pton(AF_INET, ip, &addr->sin_addr.s_addr);  // inet_pton() converts dotted-decimal ip string to network byte order.
	}
	printf("+ endpoint connection socket created.\n");
	return addr;
}


struct acceptedConn * acceptNewConn(int srv_sfd) {
	/*
	 Accept a client connection using accept().
	 This functions allocates memoery for the client socket information that needs to be free
	 manually.
	 This functions returns a pointer to a structure called `acceptedConn`
	 In case of error, `acceptedConn` will store the errno number in the `error` entry.
	*/

	int sfd_client;
	struct acceptedConn *client_conn = malloc(sizeof(struct acceptedConn));  // allocate memoery for client_conn struct information.

	// accept the connection
	struct sockaddr_in client_addr;
	unsigned int clientaddr_s = sizeof(struct sockaddr_in);
	sfd_client = accept(srv_sfd, (struct sockaddr *)&client_addr, &clientaddr_s);  // creates the sfd of the client in the server side.
	if (sfd_client < 0) {
		printf("- error with accept(): %s.\n", strerror(errno));
		client_conn->error = errno;
		return client_conn;
	}

	client_conn->sfd_client = sfd_client;
	client_conn->sock_client = client_addr;
	client_conn->error = 0;

	return client_conn;
}


int listenConn(int sfd_client) {
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
		printf("+ message received: %s", buff_recv);
		fflush(stdout);

		//TODO: Check for messages starting with "//" (Commands)
		threadReplies(sfd_client, "RECEIVED", strlen("RECEIVED"), 0);
	}

	return 0;
} 


int runInThread(void *(*routine)(void *), void *routine_arg, size_t arg_size) {
	/*
	 Helper function to run any function with any argument in a sepate thread.
	 The function will return -1 on error or the thread id on success.
	*/

	// Allocate memory for the thread argument
    void *arg_copy = malloc(arg_size);
    if (!arg_copy) {
        perror("Error allocating memory");
        return -1;
    }

    // Copy the content of the argument into the allocated memory
    memcpy(arg_copy, routine_arg, arg_size);

	pthread_t id;
	if(pthread_create(&id, NULL, routine, routine_arg) < 0) {
		printf("- Error with pthread(): %s.\n", strerror(errno));
		return -1;
	}

	return id;
}


int threadConnections(int sfd_srv) {

	while (true) {
		struct acceptedConn *client_conn = acceptNewConn(sfd_srv);
		if (client_conn->error < 0) {
			printf("- Erro with AcceptNewConn(): %s.\n", strerror(errno));
			return -1;
		}
		int sfd_client = client_conn->sfd_client;
		int *arg = malloc(sizeof(int));
		*arg = sfd_client;
			
		// Run threadNewConn() from a new thread pthread_create()
		runInThread(threadNewConn, arg, sizeof(arg));
	}
}


void *threadNewConn(void *arg_sfd_client) {
	int sfd_client = *(int *)arg_sfd_client; 
	free(arg_sfd_client);

	listenConn(sfd_client);	
	close(sfd_client);

	return NULL;
}


int threadReplies(int dst_sfd, char *buffer, size_t buffer_size, int flag) {
	if (!flag) flag = 0;
	
	struct reply_info info;
	info.sockfd = dst_sfd;
	info.buffer = buffer;
	info.buffer_s = buffer_size;
	info.flags = flag;
	struct reply_info *arg = malloc(sizeof(struct reply_info));
	if (!arg) {
		printf("Error with malloc(): %s\n", strerror(errno));
		return -1;
	}
	*arg = info;
	runInThread(threadReply, arg, sizeof(arg));		
	
	return 0;
}


void *threadReply(void *arg_reply_info) {
	struct reply_info reply = *(struct reply_info *)arg_reply_info;	
	free(arg_reply_info);

	if (send(reply.sockfd, reply.buffer, reply.buffer_s, reply.flags) < 0) {
		return NULL;
	}
	
	return NULL;
}
