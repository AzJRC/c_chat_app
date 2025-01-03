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


// Function definitions
int createTCPv4Socket();
struct sockaddr_in *createV4Sock(char *ip, int port);
struct acceptedConn * acceptNewConn(int srv_sfd);
int listenConn(int sfd_client);	
void *threadNewConn(void *arg);
int threadConnections(int sfd_srv);
void *threadReply(void *arg_reply_info);
int threadReplies(int dst_sfd, char *buffer, size_t buffer_size, int flag);
int runInThread(void *(*routine)(void *), void *routine_arg, size_t arg_size);
void *threadReply(void *arg_reply_info);
int threadReplies(int dst_sfd, char *buffer, size_t buffer_size, int flag);
void log_message(LogLevel level, const char *log_msg, ...);
int _print_routine_name(const char *routine_name, void *(*routine)(void *));


#endif
