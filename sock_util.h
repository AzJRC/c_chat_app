#ifndef SOCKETUTIL_SOCKETUTIL_H

// Compatibility
#define SOCKETUTIL_SOCKETUTIL_H
#define _POSIX_C_SOURCE 200809L

//Dependencies
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <malloc.h>
#include <stdarg.h>

// Constants
#define INIT_MAXIMUM_CONN_LIST 5

// Macros
#define LOG_DEBUG_MESSAGE(msg, ...) handleLogs(DEBUG, msg, ##__VA_ARGS__)
#define LOG_INFO_MESSAGE(msg, ...) handleLogs(INFO, msg, ##__VA_ARGS__)
#define LOG_ERROR_MESSAGE(msg, ...) handleLogs(ERROR, msg, ##__VA_ARGS__)

// Type definitions
typedef enum { DEBUG, INFO, ERROR } LogLevel;


// Struct definitions
struct acceptedConn {
	int sfd_client;
	struct sockaddr_in sock_client;
	int error;
};

struct reply_info {
	int sockfd;
	char *buffer;
	size_t buffer_s;
	int flags;
};

//Mutex
static pthread_mutex_t conn_list_mutex = PTHREAD_MUTEX_INITIALIZER;


// Function definitions
void handleLogs(LogLevel level, const char *log_msg, ...);

int runTCPServer(char *srv_addr, int srv_port, int backlog);
int createTCPv4Socket();
struct sockaddr_in *createIPv4Sockaddr(char *ip, int port);
struct acceptedConn * acceptNewConn(int srv_sfd);

int listenConn(int sfd_client);	

int runInThread(void *(*routine)(void *), void *routine_arg, size_t arg_size);

int runThreadConnections(int sfd_srv);
void *threadConnections(void *arg);

int runThreadReply(int dst_sfd, char *buffer, size_t buffer_size, int flag);
void *threadReply(void *arg_reply_info);


#endif
