#include "regular_chat.h"

void run_regular_chat_server(char *port)
{
    // title
    printServerTitle();
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];

    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(port));
    server_addr.sin_addr.s_addr = INADDR_ANY;

    bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr));

    listen(server_socket, 1);

    printf(GREEN " Server started. Listening on port %s ...\n" RESET, port);

    client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);
    printf(GREEN "*** Client connected ***\n\n" RESET);

    struct timeval tv;
    fd_set read_fds;
    FD_ZERO(&read_fds);

    while (1)
    {
        FD_SET(STDIN_FILENO, &read_fds);
        FD_SET(client_socket, &read_fds);
        tv.tv_sec = 1;
        tv.tv_usec = 0;

        // select the ready descriptor - either stdin or client socket
        int activity = select(client_socket + 1, &read_fds, NULL, NULL, &tv);

        if (activity < 0)
        {
            perror("select");
            break;
        }

        if (FD_ISSET(STDIN_FILENO, &read_fds))
        {
            fgets(buffer, BUFFER_SIZE - 1, stdin);
            printf(YELLOW "You: " RESET "%s" , buffer);
            send(client_socket, buffer, strlen(buffer), 0);
        }

        if (FD_ISSET(client_socket, &read_fds))
        {
            memset(buffer, 0, BUFFER_SIZE);
            int bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);

            if (bytes_received <= 0)
            {
                printf(RED "Client disconnected... Bye Bye\n" RESET);
                close(client_socket);
                break;
            }
            else
            {
                buffer[bytes_received] = '\0';
                printf(YELLOW "Client: "RESET " %s" RESET, buffer);
            }
        }
    }

    close(server_socket);
}

void run_regular_chat_client(char *ip, char *port)
{
    // title
    printClientTitle();
    int client_socket;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    client_socket = socket(AF_INET, SOCK_STREAM, 0);

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(port));
    server_addr.sin_addr.s_addr = inet_addr(ip);

    connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr));

    printf("Connected to server %s:%s...\n", ip, port);
    printf("*** Welcome to the chat! ***\n\n");

    struct timeval tv;
    fd_set read_fds;
    FD_ZERO(&read_fds);
    while (1)
    {
        FD_SET(STDIN_FILENO, &read_fds);
        FD_SET(client_socket, &read_fds);
        tv.tv_sec = 1;
        tv.tv_usec = 0;

        int activity = select(client_socket + 1, &read_fds, NULL, NULL, &tv);

        if (activity < 0)
        {
            perror("select");
            break;
        }

        if (FD_ISSET(STDIN_FILENO, &read_fds))
        {
            fgets(buffer, BUFFER_SIZE - 1, stdin);
            printf(YELLOW "You: " RESET "%s" , buffer);
            send(client_socket, buffer, strlen(buffer), 0);
        }

        if (FD_ISSET(client_socket, &read_fds))
        {
            memset(buffer, 0, BUFFER_SIZE);
            int bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);

            if (bytes_received <= 0)
            {
                printf(RED "Server disconnected... Bye Bye\n" RESET);
                close(client_socket);
                break;
            }
            else
            {
                buffer[bytes_received] = '\0';
                printf(YELLOW "Server: "RESET "%s" , buffer); 
            }
        }
    }

    close(client_socket);
}
