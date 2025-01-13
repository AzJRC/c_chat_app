all:
	gcc client.c util.c -o ./bin/client.o
	gcc server.c util.c -o ./bin/server.o

debug:
	gcc client.c util.c -g -Wall -Wextra -o ./bin/client.o
	gcc server.c util.c -g -Wall -Wextra -o ./bin/server.o

