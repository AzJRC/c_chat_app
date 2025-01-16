/* sock_server.c */
#include "util.h"

#define SRV_ADDR ""
#define SRV_PORT 2357
#define LST_BACKLOG 2


int acceptNewConn(int srv_sfd, struct acceptedConn *client_conn);

void *listenConn(void *sfd_client_arg);

int runThreadConnections(int sfd_srv);

void broadcastMessage(char *buffer, int original_sender);

int runThreadReply(int dst_sfd, char *buffer, size_t buffer_size, int flag);


int main() {

    /*
     Starting the TCP server will do the following:
     - run createTCPv4Socket()
     - `bind()` the socket to the address SRV_ADDR and port SRV_PORT
     - `listen()` for connections using the previously created socket.
    */
    int server_sfd = runTCPServer(SRV_ADDR, SRV_PORT, LST_BACKLOG);
	if (server_sfd < 0) {
		return -1;
	}

    /*
     `runThreadConnections()` is the principal function of this
     application. It will stuck the application in a `while(true)`
     loop ti accept connections using `acceptNewConn()`.
     On each new connection, the `threadConnections()` function is
     called to handle it.
    */
	runThreadConnections(server_sfd);

	shutdown(server_sfd, SHUT_RDWR);
	LOG_INFO_MESSAGE("Application closed.\n");	

	return 0;
}

//
// CREATE THE TCP SERVER AND RUN IT
//

int runTCPServer(char *srv_addr, int srv_port, int backlog) {
	int server_sfd = createTCPv4Socket();		
	
	struct sockaddr_in *addr = createSockaddrStruct(srv_addr, srv_port);

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

//
// HANDLE THE TCP CONNECTIONS
//


//BUG We should not use global variables
struct acceptedConn connections_list[10];
int connections_count = 0;

int runThreadConnections(int sfd_srv) {	
	/*
	 Start running each client connection in a separate thread.
	 Client messages are handled within a thread in each client's 
	 corresponding thread.
	*/

	while (true) {
        // acceptNewConn
		struct acceptedConn client_conn;
        acceptNewConn(sfd_srv, &client_conn);
		if (client_conn.error < 0) {
			printf("- Error with AcceptNewConn(): %s.\n", strerror(errno));
			return -1;
		}
		LOG_DEBUG_MESSAGE("Client connection accepted: SocketFD %d.\n", client_conn.sfd_client);
		LOG_DEBUG_MESSAGE("Connections count: %d.\n", connections_count);

		connections_list[connections_count] = client_conn;
		connections_count += 1;

		// Run threadNewConn() from a new thread pthread_create() 
        pthread_t tid_connection;
        pthread_create(&tid_connection, NULL, listenConn, &client_conn.sfd_client);

		LOG_DEBUG_MESSAGE("Listening client connection in a new thread.\n");

        pthread_detach(tid_connection);
	}


    return 0;
}


int acceptNewConn(int srv_sfd, struct acceptedConn *client_conn) {
	/*
	 Accept a client connection using accept().
	 This functions allocates memoery for the client socket information that needs to be free
	 manually.
	 This functions returns a pointer to a structure called `acceptedConn`
	 In case of error, `acceptedConn` will store the errno number in the `error` entry.
	*/

	struct sockaddr_in client_addr;  // client socket information
	unsigned int clientaddr_size = sizeof(struct sockaddr_in);  // client socket size

	int sfd_client;
	sfd_client = accept(srv_sfd, (struct sockaddr *)&client_addr, &clientaddr_size);  // accept the connection
	if (sfd_client < 0) {
		LOG_ERROR_MESSAGE("Error with accept(): %s.\n", strerror(errno));
        client_conn->sfd_client = -1;
        client_conn->sock_client = client_addr;
		client_conn->error = errno;
		return 1;
	}
	client_conn->sfd_client = sfd_client;
	client_conn->sock_client = client_addr;
	client_conn->error = 0;

	return 0;
}


void *listenConn(void *sfd_client_arg) {
	/*
	 Process a connection by listening to a received accepted connection from `acceptedConn()` using
	 the `listen()^ function.
	 Loop indifinitely until the connection is closed by the client or an error arise.
	*/

    int sfd_client = *(int *)sfd_client_arg;

	char buff_recv[1024];
	ssize_t recv_content = 0;

	while (true) {
		recv_content = recv(sfd_client, buff_recv, 1024, 0);
		if (recv_content < 0) {
			printf("- error with recv(): %s.\n", strerror(errno));
			return (void *)-1;
		}

		if(recv_content == 1 || recv_content == 0) {
			break;
		}

		buff_recv[recv_content - 1] = 0;  // We assume all received messages end with a breakline \n.

		LOG_DEBUG_MESSAGE("Client [SocketFD %d] message received: ", sfd_client);
		printf("[ %s ]\n", buff_recv);

		broadcastMessage(buff_recv, sfd_client);
	}

	return (void *)0;
} 


void broadcastMessage(char *buffer, int original_sender) {
    /*
     Broadcast buffer to all connected clients except original_sender.
    */
	
	for (int i = 0; i < connections_count; i++) {
		int client_sfd = connections_list[i].sfd_client;

		if (client_sfd == original_sender) continue;
		
		send(client_sfd, buffer, strlen(buffer), 0);
	}

	return;
}

