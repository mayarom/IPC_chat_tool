#include "stnc.h"

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
    int sockfd;
    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd < 0)
    {
        print_error_and_exit("ERROR opening socket");
    }

    // Get isServer
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(port));
    int rval = inet_pton(AF_INET, ip, &serv_addr.sin_addr); // Convert IPv4 from text to binary form
    if (rval <= 0)
    {
        print_error_and_exit("ERROR inet_pton() failed");
    }
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        print_error_and_exit("ERROR connecting");
    }

    // Create pollfd to monitor stdin and socket
    struct pollfd fds[2];
    create_pollfd(sockfd, fds);

    char messageBuffer[BUFFER_SIZE_MESSAGE];
    int timeout = -1; // Infinite timeout

    while (1)
    {
        if (isTest)
        { // isTest mode
          // generate file if not exist
            generate_file("chat_test.txt", 100 * 1024 * 1024, quiet);
            // create new port for file transfer port+1
            char new_port[10];
            sprintf(new_port, "%d", atoi(port) + 1);

            // send filesize
            int filesize = find_file_size("chat_test.txt");
            char filesize_str[20];
            sprintf(filesize_str, "%d", filesize);
            int bytesSent = byte_sent(sockfd, filesize_str, strlen(filesize_str));
            sleep(0.1);

            // send checksum
            uint32_t checksum = generate_checksum("chat_test.txt", quiet);
            char checksum_str[20];
            sprintf(checksum_str, "%d", checksum);
            bytesSent = send(sockfd, checksum_str, strlen(checksum_str), 0);
            if (bytesSent < 0)
            {
                print_error_and_exit("ERROR send() failed");
            }
            sleep(0.1);

            struct timeval start;
            gettimeofday(&start, NULL);

            // send start time
            char start_time_str[20];
            sprintf(start_time_str, "%ld.%06ld", start.tv_sec, start.tv_usec);
            bytesSent = send(sockfd, start_time_str, strlen(start_time_str), 0);
            if (bytesSent < 0)
            {
                print_error_and_exit("ERROR send() failed");
            }
            sleep(0.1);

            if (isTCP)
            {
                if (ipv4)
                {
                    if (!quiet)
                        printf("TCP IPv4\n");
                    bytesSent = send(sockfd, "ipv4 isTCP", 8, 0); // send isTest command
                    send_file(ip, new_port, "chat_test.txt", AF_INET, SOCK_STREAM, IPPROTO_TCP, quiet);
                }
                else if (ipv6)
                {
                    if (!quiet)
                        printf("TCP IPv6\n");
                    bytesSent = send(sockfd, "ipv6 isTCP", 8, 0); // send isTest command
                    send_file(ip, new_port, "chat_test.txt", AF_INET6, SOCK_STREAM, IPPROTO_TCP, quiet);
                }
            }
            else if (udp)
            {
                if (ipv4)
                {
                    if (!quiet)
                        printf("UDP IPv4\n");
                    bytesSent = send(sockfd, "ipv4 udp", 8, 0); // send isTest command
                    send_file(ip, new_port, "chat_test.txt", AF_INET, SOCK_DGRAM, 0, quiet);
                }
                else if (ipv6)
                {
                    if (!quiet)
                        printf("UDP IPv6\n");
                    bytesSent = send(sockfd, "ipv6 udp", 8, 0); // send isTest command
                    send_file(ip, new_port, "chat_test.txt", AF_INET6, SOCK_DGRAM, 0, quiet);
                }
            }
            else if (uds)
            {
                run_uds_test(uds, dgram, stream, sockfd, new_port, quiet);
            }
            else if (isMmap)
            {
                if (!quiet)
                    printf("MMAP\n");
                copy_file_to_shm_mmap("chat_test.txt", filename, quiet); // copy file to shared memory
                bytesSent = send(sockfd, filename, strlen(filename), 0); // send filepath to shared memory
            }
            else if (isPipe)
            {
                if (!quiet)
                {
                }
                printf("PIPE\n");
                bytesSent = send(sockfd, filename, strlen(filename), 0); // send fifo name to named pipe
                send_file_fifo("chat_test.txt", filename, quiet);        // copy file to named pipe
            }

            if (bytesSent < 0)
            {
                printf("ERROR send() failed\n");
                exit(1);
            }

            delete_file("chat_test.txt", quiet);
            exit(0);
        }
        // Poll stdin and socket
        int pollResult = create_poll(fds);

        if (fds[0].revents & POLLIN) // check if user input
        {
            // Read user input
            int bytesRead = read_user_input(messageBuffer);

            messageBuffer[bytesRead] = '\0';

            // Send user input to isServer
            int bytesSent = send(sockfd, messageBuffer, bytesRead, 0);
            if (bytesSent < 0)
            {
                print_error_and_exit("ERROR send() failed");
            }
            bzero(messageBuffer, BUFFER_SIZE_MESSAGE);
        }
        if (fds[1].revents & POLLIN) // check if isServer sent message
        {
            // Read message from isServer
            int bytesRecv = recv(sockfd, messageBuffer, BUFFER_SIZE_MESSAGE - 1, 0);
            if (bytesRecv < 0)
            {
                print_error_and_exit("ERROR recv() failed");
            }
            if (bytesRecv == 0)
            {
                printf("Server closed connection... bye bye \n");
                exit(0);
            }
            messageBuffer[bytesRecv] = '\0';
            printf("Server: %s", messageBuffer);
            bzero(messageBuffer, BUFFER_SIZE_MESSAGE);
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
