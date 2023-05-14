#include "client.h"

// create a socket and bind it to a port
int client_create_socket(int domain, int type, int protocol) {
    int sockfd = socket(domain, type, protocol);
    if (sockfd < 0) {
        print_error_and_exit("ERROR opening socket");
    }
    return sockfd;
}

// bind a socket to a port
int client_bind_socket_to_port(int sockfd, int port, int domain, in_addr_t addr) {
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = domain;
    serv_addr.sin_addr.s_addr = addr;
    serv_addr.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        print_error_and_exit("ERROR on binding");
    }
}

// listen for connections on a socket
void client_listen_for_connections(int sockfd, int backlog) {
    if (listen(sockfd, backlog) < 0) {
        print_error_and_exit("ERROR on listen");
    }
}

// accept a connection on a socket
int client_accept_connection(int sockfd) {
    struct sockaddr_in cli_addr;
    socklen_t clilen = sizeof(cli_addr);
    int clientSock = accept(sockfd, (struct sockaddr*)&cli_addr, &clilen);
    if (clientSock < 0) {
        print_error_and_exit("ERROR on accept");
    }
    return clientSock;
}

// connect to a socket
void client_poll_for_events(struct pollfd *fds, nfds_t nfds, int timeout) {
    int pollResult = poll(fds, nfds, timeout);
    if (pollResult < 0) {
        print_error_and_exit("ERROR poll() failed");
    }
}

void client_handle_user_input(int clientSock, char *messageBuffer, int bufferSize) {
    int bytesRead = read_user_input(messageBuffer);
    messageBuffer[bytesRead] = '\0';

    int bytesSent = send(clientSock, messageBuffer, bytesRead, 0);
    if (bytesSent < 0) {
        print_error_and_exit("ERROR send() failed");
    }
    bzero(messageBuffer, bufferSize);
}

// receive data on a socket
int client_handle_client_message(int clientSock, char *messageBuffer, int bufferSize, char *port) {
    int bytesRecv = recv(clientSock, messageBuffer, bufferSize - 1, 0);
    if (bytesRecv < 0) {
        print_error_and_exit("ERROR recv() failed");
    }
    if (bytesRecv == 0) {
        printf("Server closed connection... bye bye\n");
        exit(0);
    }
    messageBuffer[bytesRecv] = '\0';
    printf("Server: %s", messageBuffer);
    bzero(messageBuffer, bufferSize);
    return bytesRecv;
}




