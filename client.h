#ifndef CLIENT_H
#define CLIENT_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>

int client_create_socket(int domain, int type, int protocol);

int client_bind_socket_to_port(int sockfd, int port, int domain, in_addr_t addr) ;

void client_listen_for_connections(int sockfd, int backlog);

int client_accept_connection(int sockfd);

void client_poll_for_events(struct pollfd *fds, nfds_t nfds, int timeout);

void client_handle_user_input(int clientSock, char *messageBuffer, int bufferSize);

int client_handle_client_message(int clientSock, char *messageBuffer, int bufferSize, char *port);

#endif
