#ifndef SOCKETUTIL_SOCKETUTIL_H

#define SOCKETUTIL_SOCKETUTIL_H
#define _POSIX_C_SOURCE 200809L


#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <malloc.h>


struct acceptedConn {
	int sfd_client;
	struct sockaddr_in sock_client;
	int error;
};


int createTCPv4Socket();
struct sockaddr_in *createV4Sock(char *ip, int port);
struct acceptedConn * acceptNewConn(int srv_sfd);
int listenConn(int sfd_client);	

#endif
