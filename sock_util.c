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
		printf("+ message received: %s\n", buff_recv);
	}

	return 0;
} 
