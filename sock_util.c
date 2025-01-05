#include "sock_util.h"


void handleLogs(LogLevel level, const char *log_msg, ...) {
	/*
	 Helper function to print to the console logging information.
	*/
		
	const char *level_str;
	switch (level) {
		case INFO:
			level_str = "+ [INFO]"; 
			break;
		case DEBUG: 
			level_str = "* [DEBUG]";
			break;
		case ERROR:
			level_str = "- [ERROR]";
			break;
	}

	va_list args;
	va_start(args, log_msg);

	printf("%s ", level_str);
	vprintf(log_msg, args);

	va_end(args);

	return;
} 


int runTCPServer(char *srv_addr, int srv_port, int backlog) {
	int server_sfd = createTCPv4Socket();		
	
	struct sockaddr_in *addr = createIPv4Sockaddr(srv_addr, srv_port);

	if (bind(server_sfd, (struct sockaddr *) addr, sizeof(*addr)) == -1) {
		printf("- error with bind(): %s.\n", strerror(errno));
		return -1;
	}
	LOG_DEBUG_MESSAGE("Address bind succesfull.\n");

	// listen for incomming connections
	if (listen(server_sfd, backlog) == -1) {
		printf("- error with listen(): %s.\n", strerror(errno));
		return -1;
	}
	LOG_DEBUG_MESSAGE("Listen for incomming connections.\n");	

	return server_sfd;
}


int createTCPv4Socket() {
	/*
	 Create a socket file descriptor using the socket() function.
	 The socket will use AF_INET for IPv4 and SOCK_STREAM for TCP.
	*/

	int sfd = socket(AF_INET, SOCK_STREAM, 0);  // create a TCP v4 socket fd.	
	if (sfd < 0) {
		LOG_ERROR_MESSAGE("Error with socket(): %s.\n", strerror(errno));
		return -1;
	}
	LOG_DEBUG_MESSAGE("IPv4 TCP socket created succesfully. SocketFD %d.\n", sfd);
	return sfd;
}


struct sockaddr_in *createIPv4Sockaddr(char *ip, int port) { 
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
	LOG_DEBUG_MESSAGE("IPv4 socket sockaddr_in for receving connections created successfully.\n");
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
		LOG_ERROR_MESSAGE("Error with accept(): %s.\n", strerror(errno));
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
	ssize_t recv_content = 0;
	while (true) {
		//BUG Verify the recv_content errno case and when recv_content = 1 || 0.

		if ( (recv_content = recv(sfd_client, buff_recv, 1024, 0)) < 0) {
			if (errno == 104) {  // if connection reset exit gracefully
				LOG_DEBUG_MESSAGE("Client disconnected. ClientFD %d.\n", sfd_client);
				break;
			}
			printf("- error with recv(): %s.\n", strerror(errno));
			return -1;
		}

		if(recv_content == 1 || recv_content == 0) {
			break;
		}

		buff_recv[recv_content - 1] = 0;  // We assume all received messages end with a breakline \n.


		LOG_DEBUG_MESSAGE("Client [SocketFD %d] message received: ", sfd_client);
		printf("[ %s ]\n", buff_recv);

		//TODO: Check for messages starting with "//" (Commands)
		// runThreadReplies(sfd_client, "RECEIVED", strlen("RECEIVED"), 0);
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
		LOG_ERROR_MESSAGE("Error with malloc(): %s.\n", strerror(errno));
        return -1;
    }

    // Copy the content of the argument into the allocated memory
    memcpy(arg_copy, routine_arg, arg_size);

	pthread_t id;
	if(pthread_create(&id, NULL, routine, arg_copy) < 0) {
		LOG_ERROR_MESSAGE("Error with pthread(): %s.\n", strerror(errno));
		return -1;
	}
	LOG_DEBUG_MESSAGE("Thread created succesfully: ID %02x.\n", id);

	return id;
}


int runThreadConnections(int sfd_srv) {	
	/*
	 Start running each client connection in a separate thread.
	 Client messages are handled within a thread in each client's 
	 corresponding thread.
	*/

	while (true) {
		struct acceptedConn *client_conn = acceptNewConn(sfd_srv);
		if (client_conn->error < 0) {
			printf("- Error with AcceptNewConn(): %s.\n", strerror(errno));
			return -1;
		}
		LOG_DEBUG_MESSAGE("Client connection accepted: SocketFD %d.\n", client_conn.sfd_client);

		int sfd_client = client_conn->sfd_client;
		int *sfd_client_ptr = &sfd_client;
			
		// Run threadNewConn() from a new thread pthread_create()
		LOG_DEBUG_MESSAGE("Calling runInThead() from runThreadConnections().\n");
		runInThread(threadConnections, sfd_client_ptr, sizeof(sfd_client));
	}
}


void *threadConnections(void *arg_sfd_client) {
	int sfd_client = *(int *)arg_sfd_client; 
	free(arg_sfd_client);

	listenConn(sfd_client);	
	close(sfd_client);

	return NULL;
}


int runThreadReply(int dst_sfd, char *buffer, size_t buffer_size, int flag) {
	if (!flag) flag = 0;
	
	struct reply_info info;
	info.sockfd = dst_sfd;
	info.buffer = buffer;
	info.buffer_s = buffer_size;
	info.flags = flag;
	struct reply_info *arg = malloc(sizeof(struct reply_info));
	if (!arg) {
		LOG_ERROR_MESSAGE("Error with malloc(): %s.\n", strerror(errno));
		return -1;
	}
	*arg = info;
	LOG_DEBUG_MESSAGE("Calling runInThead() from runThreadReplies().\n");
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


