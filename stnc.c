#include "stnc.h"
#include <string.h>

int udp, uds, dgram, stream, isMmap, isPipe, deleteFile, quiet;
char *filename, *ip, *port, *type, *param;
int isClient, isServer, isTest, ipv4, ipv6, isTCP, isUDP, isUDS, isDgram, isStream, isMmapEnabled, isPipeEnabled, isDeleteFile, isQuietMode;
char *fileName, *ipAddress, *portNumber, *connectionType, *parameter;

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        print_usage();
        return 1;
    }

    initialize_variables(&isClient, &isServer, &isTest, &quiet, &ip, &port, &type, &param);

    parse_arguments(argc, argv, &isClient, &isServer, &isTest, &quiet, &ip, &port, &type, &param);

    int result = run_test_client(isTest, isClient, type, param, &ipv4, &ipv6, &uds, &isMmap, &isPipe, &isTCP, &udp, &dgram, &stream, &filename);
    if (result != 0)
    {
        return 1;
    }

    run_program(isClient, isServer, ip, port, print_usage);

    return 0;
}

void client_main_func(char *ip, char *port)
{
    if (!quiet)
    {
        printClientTitle();
        // connection info
        printf("IP: %s\n", ip);
        printf("Port: %s\n", port);
    }

    // Create socket
    int sockfd = client_create_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    // Get server address
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(port));
    int rval = inet_pton(AF_INET, ip, &serv_addr.sin_addr); // Convert IPv4 from text to binary form
    if (rval <= 0)
    {
        print_error_and_exit("ERROR inet_pton() failed");
    }

    // Connect to the server
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        print_error_and_exit("ERROR connecting");
    }

    // Create pollfd to monitor stdin and socket
    struct pollfd fds[2];
    fds[0].fd = STDIN_FILENO;
    fds[0].events = POLLIN;
    fds[1].fd = sockfd;
    fds[1].events = POLLIN;

    char messageBuffer[BUFFER_SIZE_MESSAGE];
    int timeout = -1; // Infinite timeout

    while (1)
    {
        // Poll stdin and socket
        client_poll_for_events(fds, 2, timeout);

        if (fds[0].revents & POLLIN)
        {
            // User input
            client_handle_user_input(sockfd, messageBuffer, BUFFER_SIZE_MESSAGE);
        }

        if (fds[1].revents & POLLIN)
        {
            // Message from server
            int bytesRecv = client_handle_client_message(sockfd, messageBuffer, BUFFER_SIZE_MESSAGE, port);
            if (bytesRecv == 0)
            {
                printf("Server closed connection... bye bye\n");
                exit(0);
            }
        }
    }

    close(sockfd);
}
void run_program(int isClient, int isServer, char *ip, char *port, void (*print_usage)())
{
    if (isClient)
    {
        client_main_func(ip, port);
    }
    else if (isServer)
    {
        server_main_func(port);
    }
    else
    {
        print_usage();
    }
}
void run_uds_test(int uds, int dgram, int stream, int sockfd, char *new_port, int quiet)
{
    int bytesSent;

    if (uds)
    {
        if (dgram)
        {
            if (!quiet)
                printf("UDS DGRAM\n");
            bytesSent = send(sockfd, "uds dgram", 9, 0); // send isTest command
            sleep(0.1);
            send_file(0, new_port, "chat_test.txt", AF_UNIX, SOCK_DGRAM, 0, quiet);
        }
        else if (stream)
        {
            if (!quiet)
                printf("UDS STREAM\n");
            bytesSent = send(sockfd, "uds stream", 10, 0); // send isTest command
            sleep(0.1);
            send_file(0, new_port, "chat_test.txt", AF_UNIX, SOCK_STREAM, 0, quiet);
        }
    }
}

void server_main_func(char *port)
{
    // Print server header
    if (!quiet)
    {
        printServerTitle();
        printf("Server listening on port %s\n", port);
        printf("Press Ctrl+C to exit\n");
    }

    // Create socket
    int sockfd = create_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    // Bind socket to port
    bind_socket_to_port(sockfd, AF_INET, INADDR_ANY, atoi(port));

    // Main server loop
    while (1)
    {
        // Listen for connections
        listen_for_connections(sockfd, 1);

        // Accept connection
        int clientSock = accept_connection(sockfd);

        if (!quiet)
            printf("Client connected\n");

        // Create poll
        struct pollfd fds[2] = {
            {.fd = STDIN_FILENO, .events = POLLIN},
            {.fd = clientSock, .events = POLLIN}};

        char messageBuffer[BUFFER_SIZE_MESSAGE];

        // Connection loop
        while (1)
        {
            // Poll for events
            poll_for_events(fds, 2, -1);

            // Check for user input
            if (fds[0].revents & POLLIN)
            {
                handle_user_input(clientSock, messageBuffer, BUFFER_SIZE_MESSAGE);
            }

            // Check for client message
            if (fds[1].revents & POLLIN)
            {
                if (handle_client_message(clientSock, messageBuffer, BUFFER_SIZE_MESSAGE, port))
                {
                    break;
                }
            }
        }
        close(clientSock);
    }
    close(sockfd);
}

void printServerTitle()
{
    int terminalWidth = 65; // Set the default terminal width

    // Print the isServer title
    printf("╔");
    for (int i = 0; i < terminalWidth - 2; i++)
        printf("═");
    printf("╗\n");
    printf("║");
    for (int i = 0; i < (terminalWidth - 22) / 2; i++) // Adjust for the title length
        printf(" ");
    printf("Server Chat Side");
    for (int i = 0; i < (terminalWidth - 22) / 2; i++) // Adjust for the title length
        printf(" ");
    printf("    ║\n");

    printf("╚");
    for (int i = 0; i < terminalWidth - 2; i++)
        printf("═");
    printf("╝\n");
}

void parse_arguments(int argc, char *argv[], int *isClient, int *isServer, int *isTest, int *quiet, char **ip, char **port, char **type, char **param)
{
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-c") == 0)
        {
            *isClient = 1;
            *ip = argv[++i];
            *port = argv[++i];
        }
        else if (strcmp(argv[i], "-s") == 0)
        {
            *isServer = 1;
            *port = argv[++i];
        }
        else if (strcmp(argv[i], "-p") == 0)
        {
            *isTest = 1;
            *type = argv[++i];
            *param = argv[++i];
        }
        else if (strcmp(argv[i], "-q") == 0)
        {
            *quiet = 1;
        }
    }
}

void printClientTitle()
{
    int terminalWidth = 65; // Set the default terminal width

    // Print the isClient title
    printf("╔");
    for (int i = 0; i < terminalWidth - 2; i++)
        printf("═");
    printf("╗\n");

    printf("║");
    for (int i = 0; i < (terminalWidth - 22) / 2; i++) // Adjust for the title length
        printf(" ");
    printf("Client Chat Side");
    for (int i = 0; i < (terminalWidth - 22) / 2; i++) // Adjust for the title length
        printf(" ");
    printf("║\n");

    printf("╚");
    for (int i = 0; i < terminalWidth - 2; i++)
        printf("═");
    printf("╝\n");
}
void print_usage()
{
    printf("╔══════════════════════════════════════════════════════╗\n");
    printf("║                       Usage                          ║\n");
    printf("╠══════════════════════════════════════════════════════╣\n");
    printf("║ Part A:                                              ║\n");
    printf("║   Client: ./stnc -[c] <ip> <port>                    ║\n");
    printf("║   Server: ./stnc -[s] <port>                         ║\n");
    printf("║ Part B:                                              ║\n");
    printf("║   Client: ./stnc -[c] <ip> <port> -[p] <type> <param>║\n");
    printf("╚══════════════════════════════════════════════════════╝\n");
}
int create_poll(int sockfd)
{
    struct pollfd fds[2] = {
        {.fd = STDIN_FILENO, .events = POLLIN},
        {.fd = sockfd, .events = POLLIN}};
    return fds;
}

int read_user_input(char *messageBuffer)
{
    // read user input
    int bytesRead = read(STDIN_FILENO, messageBuffer, BUFFER_SIZE_MESSAGE);
    // check if read failed
    if (bytesRead < 0)
    {
        printf("ERROR read() failed\n");
        exit(1);
    }
    return bytesRead;
}
int byte_sent(int sockfd, char *messageBuffer, int messageSize)
{
    // send message to isServer
    int bytesSent = send(sockfd, messageBuffer, messageSize, 0);
    // check if send failed
    if (bytesSent < 0)
    {
        printf("ERROR send() failed\n");
        exit(1);
    }
    return bytesSent;
}

void create_pollfd(int sockfd, struct pollfd *fds)
{
    fds[0].fd = STDIN_FILENO;
    fds[0].events = POLLIN;
    fds[1].fd = sockfd;
    fds[1].events = POLLIN;
}

int is_quit(char *messageBuffer)
{
    return !strcmp(messageBuffer, "quit\n");
}

void print_error_and_exit(char *message)
{
    printf("%s\n", message);
    exit(1);
}

int find_file_size(char *fileName)
{
    FILE *file = fopen(fileName, "r");
    if (file == NULL)
    {
        printf("ERROR fopen() failed\n");
        exit(1);
    }
    fseek(file, 0L, SEEK_END);
    int fileSize = ftell(file);
    fclose(file);
    return (int)fileSize;
}
