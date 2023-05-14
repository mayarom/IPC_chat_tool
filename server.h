#ifndef SERVER_H
#define SERVER_H

#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netinet/tcp.h>
#include <sys/time.h>
#include <poll.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "globals.h"
#include "server.h"


// create a socket and bind it to a port
int create_socket(int domain, int type, int protocol);
// bind a socket to a port
void bind_socket_to_port(int sockfd, int domain, in_addr_t addr, int port);
// listen for connections on a socket
void listen_for_connections(int sockfd, int backlog);
// accept a connection on a socket
int accept_connection(int sockfd);
// connect to a socket
void poll_for_events(struct pollfd *fds, nfds_t nfds, int timeout);
void handle_user_input(int clientSock, char *messageBuffer, int bufferSize);
// receive data on a socket
int handle_client_message(int clientSock, char *messageBuffer, int bufferSize, char *port);

#endif
