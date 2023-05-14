#include "server.h"

int create_socket(int domain, int type, int protocol)
{
    int sockfd = socket(domain, type, protocol);
    if (sockfd < 0)
    {
        perror("ERROR opening socket");
        exit(EXIT_FAILURE);
    }
    return sockfd;
}

void bind_socket_to_port(int sockfd, int domain, in_addr_t addr, int port)
{
    struct sockaddr_in serv_addr = {
        .sin_family = domain,
        .sin_addr.s_addr = addr,
        .sin_port = htons(port)};

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("ERROR on binding");
        exit(EXIT_FAILURE);
    }
}

void listen_for_connections(int sockfd, int backlog)
{
    if (listen(sockfd, backlog) < 0)
    {
        perror("ERROR on listen");
        exit(EXIT_FAILURE);
    }
}

int accept_connection(int sockfd)
{
    struct sockaddr_in cli_addr;
    socklen_t clilen = sizeof(cli_addr);
    int clientSock = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);

    if (clientSock < 0)
    {
        perror("ERROR on accept");
        exit(EXIT_FAILURE);
    }

    return clientSock;
}

void poll_for_events(struct pollfd *fds, nfds_t nfds, int timeout)
{
    int pollResult = poll(fds, nfds, timeout);
    if (pollResult < 0)
    {
        perror("ERROR poll() failed");
        exit(EXIT_FAILURE);
    }
}

void handle_user_input(int clientSock, char *messageBuffer, int bufferSize)
{
    int bytesRead = read(STDIN_FILENO, messageBuffer, bufferSize);
    if (bytesRead < 0)
    {
        perror("ERROR read() failed");
        exit(EXIT_FAILURE);
    }
    messageBuffer[bytesRead] = '\0';

    if (send(clientSock, messageBuffer, bytesRead, 0) < 0)
    {
        perror("ERROR send() failed");
        exit(EXIT_FAILURE);
    }
    bzero(messageBuffer, bufferSize);
}

int handle_client_message(int clientSock, char *messageBuffer, int bufferSize, char *port)
{
    int bytesRecv = recv(clientSock, messageBuffer, bufferSize - 1, 0);
    if (bytesRecv < 0)
    {
        perror("ERROR recv() failed");
        exit(EXIT_FAILURE);
    }
    if (bytesRecv == 0)
    {
        if (!quiet)
            printf("Client disconnected\n");
        return 1;
    }
    messageBuffer[bytesRecv] = '\0';

    if (!quiet)
        printf("Client: %s", messageBuffer);
    return 0;
}
