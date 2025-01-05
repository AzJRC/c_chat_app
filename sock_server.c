/* sock_server.c */
#include "sock_util.h"


#define SRV_ADDR ""
#define SRV_PORT 2357
#define LST_BACKLOG 2

int main() {

	int server_sfd;
	if ((server_sfd = runTCPServer(SRV_ADDR, SRV_PORT, LST_BACKLOG)) < 0) {
		return -1;
	}

	runThreadConnections(server_sfd);

	shutdown(server_sfd, SHUT_RDWR);
	LOG_INFO_MESSAGE("Application closed.\n");	

	return 0;
}
