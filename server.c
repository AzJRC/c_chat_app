/* sock_server.c */
#include "util.h"

#define SRV_ADDR ""
#define SRV_PORT 2357
#define LST_BACKLOG 2
#define DEFAULT_CONNMANAGER_INIT_CAPACITY 1

pthread_mutex_t conn_manager_mutex;

typedef struct {
    struct acceptedConn *conn_list;
    int conn_count;
    int capacity;
} connManager;

typedef struct {
    connManager *conn_manager;
    struct acceptedConn client_conn;
} listenConnClientWrapper;

int runThreadConnections(int sfd_srv);
int acceptNewConn(int srv_sfd, struct acceptedConn *client_conn);
void *listenConn(void *sfd_client_arg);
void broadcastMessage(char *buffer, int original_sender, connManager *conn_manager);


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

int initConnectionsManager(connManager *conn_manager, int conn_capacity) {
    conn_manager->conn_list = malloc(sizeof(struct acceptedConn)*conn_capacity);
    conn_manager->conn_count = 0;
    conn_manager->capacity = conn_capacity;
    return 0;
}

int increaseConnectionsManagerCapacity(connManager *conn_manager) {
    if (conn_manager->capacity < 1) conn_manager->capacity = 1;

    LOG_DEBUG_MESSAGE("Connections Manager requested an increase capacity.\n");
    int new_capacity = conn_manager->capacity * 2; 

    struct acceptedConn *new_list = realloc(
            conn_manager->conn_list,
            sizeof(struct acceptedConn) * new_capacity);
    if (!new_list) return -1;

    conn_manager->conn_list = new_list;
    conn_manager->capacity = new_capacity;
    LOG_DEBUG_MESSAGE("Connections Manager capacity is now %d.\n", conn_manager->capacity);
    return 0;
}

int addConnectionToConnectionsManager(
        connManager *conn_manager, 
        struct acceptedConn *client_conn) {
    pthread_mutex_lock(&conn_manager_mutex);

    //resize conn_manager if needed
    if (conn_manager->conn_count >= conn_manager->capacity) {
        int increase_status = increaseConnectionsManagerCapacity(conn_manager);
        if (increase_status < 0) {
            pthread_mutex_unlock(&conn_manager_mutex);
            return -1;
        } 
    }

    conn_manager->conn_list[conn_manager->conn_count] = *client_conn;
    conn_manager->conn_count += 1;

    pthread_mutex_unlock(&conn_manager_mutex);
    LOG_DEBUG_MESSAGE("Connections Manager added a new connection to the list.\n");
    return 0;
}


int runThreadConnections(int sfd_srv) {	
	/*
	 Start running each client connection in a separate thread.
	 Client messages are handled within a thread in each client's 
	 corresponding thread.
	*/

    connManager conn_manager;
    initConnectionsManager(&conn_manager, DEFAULT_CONNMANAGER_INIT_CAPACITY);

	while (true) {
		struct acceptedConn client_conn;
        acceptNewConn(sfd_srv, &client_conn);
		if (client_conn.error < 0) {
			printf("- Error with AcceptNewConn(): %s.\n", strerror(errno));
			return -1;
		}
		LOG_DEBUG_MESSAGE("Client connection accepted.\n");

        addConnectionToConnectionsManager(&conn_manager, &client_conn);

        // create listenConn arg wrapper
        listenConnClientWrapper *arg = malloc(sizeof(listenConnClientWrapper));
        arg->conn_manager = &conn_manager;
        arg->client_conn = client_conn;

		// Run threadNewConn() from a new thread pthread_create() 
        pthread_t tid_connection;
        pthread_create(&tid_connection, NULL, listenConn, arg);
        pthread_detach(tid_connection);  // TODO change to pthread_join.

		LOG_DEBUG_MESSAGE("Connections count: [%d]\n", conn_manager.conn_count);
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


void *listenConn(void *listenConn_wrapper_arg) {
	/*
	 Process a connection by listening to a received accepted connection from `acceptedConn()` using
	 the `listen()^ function.
	 Loop indifinitely until the connection is closed by the client or an error arise.
	*/

    listenConnClientWrapper *client_wrapper = (listenConnClientWrapper *)listenConn_wrapper_arg;
    connManager *conn_manager = client_wrapper->conn_manager;
    struct acceptedConn client_conn = client_wrapper->client_conn;
    int sfd_client = client_conn.sfd_client;

    free(listenConn_wrapper_arg);

	LOG_DEBUG_MESSAGE("Listening client connection in a new thread.\n");

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

		broadcastMessage(buff_recv, sfd_client, conn_manager);
	}

	return (void *)0;
} 


void broadcastMessage(char *buffer, int original_sender, connManager *conn_manager) {
    /*
     Broadcast buffer to all connected clients except original_sender.
    */
	
    pthread_mutex_lock(&conn_manager_mutex);
	for (int i = 0; i < conn_manager->conn_count; i++) {
		int client_sfd = conn_manager->conn_list[i].sfd_client;

		if (client_sfd == original_sender) continue;
		
		send(client_sfd, buffer, strlen(buffer), 0);
	}
    pthread_mutex_unlock(&conn_manager_mutex);

	return;
}

