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


struct sockaddr_in *createSockaddrStruct(char *ip, int port) {
	/*
	 Create a IPv4 socket address using `sockaddr_in`.
	 This function allocated memory for the `sockaddr_in`
     structure that needs to be free manually.
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


int runInThread(void *(*routine)(void *), void *routine_arg, size_t arg_size) {
	/*
	 Helper function to run any function with any argument in a sepate thread.
	 The function will return -1 on error or the thread id on success.
	*/

	// Allocate memory for the thread argument
    void *routine_arg_copy = malloc(arg_size);
    if (!routine_arg_copy) {
		LOG_ERROR_MESSAGE("Error with malloc(): %s.\n", strerror(errno));
        return -1;
    }

    // Copy the content of the argument into the allocated memory
    memcpy(routine_arg_copy, routine_arg, arg_size);

	pthread_t tid = pthread_create(&tid, NULL, routine, routine_arg_copy);
	if(tid < 0) {
		LOG_ERROR_MESSAGE("Error with pthread(): %s.\n", strerror(errno));
		return -1;
	}
	LOG_DEBUG_MESSAGE("Thread created succesfully: ID %02x.\n", tid);

	return tid;
}


