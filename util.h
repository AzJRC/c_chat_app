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


// Macros
#define LOG_DEBUG_MESSAGE(msg, ...) handleLogs(DEBUG, msg, ##__VA_ARGS__)
#define LOG_INFO_MESSAGE(msg, ...) handleLogs(INFO, msg, ##__VA_ARGS__)
#define LOG_ERROR_MESSAGE(msg, ...) handleLogs(ERROR, msg, ##__VA_ARGS__)


typedef enum { DEBUG, INFO, ERROR } LogLevel;

// struct reply_info {
// 	int sockfd;
// 	char *buffer;
// 	size_t buffer_s;
// 	int flags;
// };

// Function definitions
void handleLogs(LogLevel level, const char *log_msg, ...);
int createTCPv4Socket();
struct sockaddr_in *createSockaddrStruct(char *ip, int port);
int runInThread(void *(*routine)(void *), void *routine_arg, size_t arg_size);

#endif
