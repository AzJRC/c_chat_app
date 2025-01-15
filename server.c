/* sock_server.c */
#include "util.h"

#define SRV_ADDR ""
#define SRV_PORT 2357
#define LST_BACKLOG 2

#define CONNECTION_MANAGER_INIT_CAPACITY 1
#define CONNECTION_MANAGER_DEFAULT_INCREASE_FACTOR 2


typedef struct {
	int sfd_client;
	struct sockaddr_in sock_client;
	int error;
} accepted_conns;

typedef struct {
    accepted_conns *list;
    int count;
    int capacity;
} ConnectionManager;


struct thread_client_connection_info {
    ConnectionManager *manager;
    accepted_conns *client;
};


int runTCPServer(char *srv_addr, int srv_port, int backlog);
int runThreadConnections(int sfd_srv);
int acceptNewConn(int srv_sfd, accepted_conns *client_conn);
int listenConn(accepted_conns *client_conn_info);
void broadcastMessage(char *buffer, int original_sender);


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


void initConnectionManager(ConnectionManager *manager) {
    manager->list = malloc(sizeof(accepted_conns) * CONNECTION_MANAGER_INIT_CAPACITY);
    manager->count = 0;
    manager->capacity = CONNECTION_MANAGER_INIT_CAPACITY < 0 ? 0 : CONNECTION_MANAGER_INIT_CAPACITY;
    return;
}


int increaseConnectionList(ConnectionManager *manager, int increase_factor) {
    if (manager->capacity == 0) manager->capacity += 1;
    int new_capacity = manager->capacity * increase_factor; 
    accepted_conns *new_list = realloc(manager->list, new_capacity * sizeof(accepted_conns));
    if (new_list == NULL) {
        LOG_ERROR_MESSAGE("Error with realloc(): [%s]\n", strerror(errno));
        return -1;
    }
    manager->list = new_list;
    manager->capacity = new_capacity; 
    return 0;
}

pthread_mutex_t connection_list_lock;

int addToConnectionList(ConnectionManager *manager, accepted_conns *client) {

    pthread_mutex_lock(&connection_list_lock);

    if (manager->count >= manager->capacity) {
        int increase_status = increaseConnectionList(manager, CONNECTION_MANAGER_DEFAULT_INCREASE_FACTOR);
        if (increase_status < 0) {
            LOG_ERROR_MESSAGE("Error with increaseConnectionList().\n");
            return -1;
        }
    }
    manager->list[manager->count] = *client;
    manager->count++;

    pthread_mutex_unlock(&connection_list_lock);
  
    return 0;
}


void removeFromConnectionList(ConnectionManager *manager, int sfd) {

    pthread_mutex_lock(&connection_list_lock);

    for (int i = 0; i < manager->count; i++) {
        if (manager->list[i].sfd_client != sfd) continue;

        for (int j = i; i < manager->count - 1; j++) {
            manager->list[j] = manager->list[j+1];
        }
        manager->count--;
        break;
    }

    pthread_mutex_unlock(&connection_list_lock);

    return;
}


void freeConnectionManager(ConnectionManager *manager) {
    free(manager->list);
    manager->count = 0;
    manager->capacity = 0;
    manager->list = NULL;
    return;
}


ConnectionManager conn_manager;
int runThreadConnections(int sfd_srv) {	
	/*
	 Start running each client connection in a separate thread.
	 Client messages are handled within a thread in each client's 
	 corresponding thread.
	*/
    
    initConnectionManager(&conn_manager);

	while (true) {
        // acceptNewConn
        accepted_conns client;
		acceptNewConn(sfd_srv, &client);
		if (client.error < 0) {
			printf("- Error with AcceptNewConn(): %s.\n", strerror(errno));
			return -1;
		}
		// connections_list[connections_count] = *client_conn;
		// connections_count += 1;
        addToConnectionList(&conn_manager, &client);

		LOG_DEBUG_MESSAGE("Client connection accepted: SocketFD %d.\n", client.sfd_client);
		LOG_DEBUG_MESSAGE("Connections count: %d.\n", conn_manager.count);

		// int sfd_client = client_conn->sfd_client;
		// int *sfd_client_ptr = &sfd_client;

        // struct thread_client_connection_info client_conn_info;
        // client_conn_info.manager = &conn_manager;
        // client_conn_info.client = &client;

        pthread_t id;
        pthread_create(&id, NULL, (void *)(listenConn), &client);
	}

    freeConnectionManager(&conn_manager);
}


int acceptNewConn(int srv_sfd, accepted_conns *client_conn) {
	/*
	 Accept a client connection using accept().
	 This functions allocates memoery for the client socket information that needs to be free
	 manually.
	 This functions returns a pointer to a structure called `accepted_conns`
	 In case of error, `accepted_conns` will store the errno number in the `error` entry.
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
		return -1;
	}
	client_conn->sfd_client = sfd_client;
	client_conn->sock_client = client_addr;
	client_conn->error = 0;
	return 0;
}


int listenConn(accepted_conns *client_conn) {
	/*
	 Process a connection by listening to a received accepted connection from `accepted_conns()` using
	 the `listen()^ function.
	 Loop indifinitely until the connection is closed by the client or an error arise.
	*/

	char buff_recv[1024];
	ssize_t recv_content = 0;

	while (true) {
		recv_content = recv(client_conn->sfd_client, buff_recv, 1024, 0);
		if (recv_content < 0) {
			printf("- error with recv(): %s.\n", strerror(errno));
			return -1;
		}
		buff_recv[recv_content] = '\0';  
        
        if (strcmp(buff_recv, "\n") == 0) {
            LOG_INFO_MESSAGE("Client has terminated the connection.\n");
            break;
        }


        buff_recv[recv_content - 1] = '\0';
        LOG_DEBUG_MESSAGE("Client [SocketFD %d] message received: [%s]\n", 
                client_conn->sfd_client, 
                buff_recv);

        // broadcastMessage(buff_recv, client_conn->sfd_client);
	}

    close(client_conn->sfd_client);
	return 0;
} 


void broadcastMessage(char *buffer, int original_sender) {
    /*
     Broadcast buffer to all connected clients except original_sender.
    */
	
	for (int i = 0; i < conn_manager.count; i++) {
		int client_sfd = conn_manager.list[i].sfd_client;

		if (client_sfd == original_sender) continue;
		
		send(client_sfd, buffer, strlen(buffer), 0);
	}

	return;
}

