# C Chat Application

Socket Programming in C: Group Chat Application with Multiple Users (Multithreaded Application)

# How to use?

Start by compiling the program using the command `make`. You can also run `make debug` to compile the program with debugging symbols.

Then, run the server by issuing the command `./bin/server.o`. Once the server is running, you can start clients in a similar way with `./bin/client.o`. Clients will not start if the server is not running in first place.

You can type any message in a client's terminal and the server will broadcast the message to all remaining clients. Look at the example bellow. Each member of the chat application is exected in a different terminal:

```bash
# Running the server
user@computer:~/User/Projects/c_chat_app$ ./bin/server.o 
* [DEBUG] IPv4 TCP socket created succesfully. SocketFD 3.
* [DEBUG] IPv4 socket sockaddr_in for receving connections created successfully.
* [DEBUG] Address bind succesfull.
* [DEBUG] Listen for incomming connections.
* [DEBUG] Client connection accepted: SocketFD 4.
* [DEBUG] Connections count: 1.
* [DEBUG] Calling runInThead() from runThreadConnections().
* [DEBUG] Thread created succesfully: ID 00.
* [DEBUG] Client connection accepted: SocketFD 5.
* [DEBUG] Connections count: 2.
* [DEBUG] Calling runInThead() from runThreadConnections().
* [DEBUG] Thread created succesfully: ID 00.
* [DEBUG] Client [SocketFD 4] message received: [ Hello ]  # Client 1 message
* [DEBUG] Client [SocketFD 5] message received: [ Hello 2 ]  # Client 2 message


# Running client 1 in another terminal
user@computer:~/User/Projects/c_chat_app$ ./bin/client.o 
* [DEBUG] IPv4 TCP socket created succesfully. SocketFD 3.
* [DEBUG] IPv4 socket sockaddr_in for receving connections created successfully.
Hello
+ [INFO] Message received: [ Hello 2 ]


# Runnign client 2 in another terminal
user@computer:~/User/Projects/c_chat_app$ ./bin/client.o
* [DEBUG] IPv4 TCP socket created succesfully. SocketFD 3.
* [DEBUG] IPv4 socket sockaddr_in for receving connections created successfully.
+ [INFO] Message received: [ Hello ]
Hello 2
```

To end the communication in a client press `<Enter>` without sending any message and it will be considered as a command to end the connection for that particular client.
To finish the server press `<Ctrl>+C` to force the application to terminate.


