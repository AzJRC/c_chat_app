all:
		gcc sock_client.c sock_util.c -o ./bin/sock_client.o 
		gcc sock_server.c sock_util.c -o ./bin/sock_server.o
