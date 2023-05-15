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

    // if there isTest = false
    if (!isTest)
    {
        run_program_chat(isClient, isServer, ip, port);
    }

    int result = run_test_client(isTest, isClient, type, param, &ipv4, &ipv6, &uds, &isMmap, &isPipe, &isTCP, &udp, &dgram, &stream, &filename);
    if (result != 0)
    {
        return 1;
    }

    run_program(isClient, isServer, ip, port, print_usage);

    return 0;
}
void run_program(int isClient, int isServer, char *ip, char *port, void (*print_usage)(void))
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

void client_main_func(char *ip, char *port)
{
    if (!quiet)
    {
        printClientTitle();
        printf("IP: %s\n", ip);
        printf("Port: %s\n", port);
        printf("Type: %s\n", type);
        printf("Param: %s\n", param);
        printf("Press Ctrl+C to exit\n");
    }

    // Create socket
    int sockfd = client_create_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    // Get server address
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(port));
    int rval = inet_pton(AF_INET, ip, &serv_addr.sin_addr);
    if (rval <= 0)
    {
        print_error_and_exit("ERROR inet_pton() failed");
    }

    // Connect to the server
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        print_error_and_exit("ERROR connecting");
    }

    if (!quiet)
    {
        // Request to send the large file
        char request[] = "start to send...";
        send(sockfd, request, strlen(request), 0);
    }

    // Receive the large file and measure time
    char *large_filename = "received_large_file.bin";
    struct timespec start_time, end_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    receive_large_file(sockfd, "received_large_file.bin");

    clock_gettime(CLOCK_MONOTONIC, &end_time);

    close(sockfd);

    // Calculate elapsed time
    double elapsed_time = (end_time.tv_sec - start_time.tv_sec) * 1e3 + (end_time.tv_nsec - start_time.tv_nsec) / 1e6; // milliseconds
    printf("Received file in %.2f ms\n", elapsed_time);
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

        // Send large file and measure time
        char *large_filename = "large_file.bin";
        struct timespec start_time, end_time;
        clock_gettime(CLOCK_MONOTONIC, &start_time);
        send_large_file("large_file.bin", sockfd);

        clock_gettime(CLOCK_MONOTONIC, &end_time);

        close(clientSock);

        // Calculate elapsed time
        double elapsed_time = (end_time.tv_sec - start_time.tv_sec) + (end_time.tv_nsec - start_time.tv_nsec) / 1e9;
        printf("Time elapsed: %.6f seconds\n", elapsed_time);
    }

    close(sockfd);
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
    int terminalWidth = 61; // Set the default terminal width

    // Print the Client title
    printf(CYAN "╔");
    for (int i = 0; i < terminalWidth - 2; i++)
        printf("═");
    printf("╗\n");

    printf("║");
    for (int i = 0; i < (terminalWidth - 24) / 2; i++) // Adjust for the title length
        printf(" ");
    printf(WHITE "   Client Chat Side   ");
    for (int i = 0; i < (terminalWidth - 24) / 2; i++) // Adjust for the title length
        printf(" ");
    printf(CYAN "║\n");

    printf("╚");
    for (int i = 0; i < terminalWidth - 2; i++)
        printf("═");
    printf("╝\n" RESET);
}

void printServerTitle()
{
    int terminalWidth = 61; // Set the default terminal width

    // Print the Server title
    printf(GREEN "╔");
    for (int i = 0; i < terminalWidth - 2; i++)
        printf("═");
    printf("╗\n");
    printf("║");
    for (int i = 0; i < (terminalWidth - 24) / 2; i++) // Adjust for the title length
        printf(" ");
    printf(WHITE "    Server Chat Side    ");
    for (int i = 0; i < (terminalWidth - 24) / 2; i++) // Adjust for the title length
        printf(" ");
    printf(GREEN "║\n");

    printf("╚");
    for (int i = 0; i < terminalWidth - 2; i++)
        printf("═");
    printf("╝\n" RESET);
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
    printf("║             Server: ./stnc -[s] <port> -[p] -[q]     ║\n");
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

void create_large_file(char *filename, int size_in_mb)
{
    FILE *file = fopen(filename, "wb");
    if (file == NULL)
    {
        printf("Error: Unable to create file %s\n", filename);
        exit(1);
    }

    int chunk_size = 1024 * 1024; // 1 MB
    char *buffer = malloc(chunk_size);

    if (buffer == NULL)
    {
        printf("Error: Unable to allocate memory for buffer\n");
        fclose(file);
        exit(1);
    }

    // Fill the buffer with random data
    for (int i = 0; i < chunk_size; i++)
    {
        buffer[i] = rand() % 256;
    }

    // Write buffer to file multiple times to reach the desired size
    for (int i = 0; i < size_in_mb; i++)
    {
        fwrite(buffer, 1, chunk_size, file);
    }

    fclose(file);
    free(buffer);
}
void send_large_file(const char *file_name, int sockfd)
{
    FILE *file = fopen(file_name, "rb");
    if (file == NULL)
    {
        perror("Error opening file");
        return;
    }

    char buffer[1024];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0)
    {
        if (send(sockfd, buffer, bytes_read, 0) == -1)
        {
            perror("Error sending file");
            fclose(file);
            return;
        }
    }

    fclose(file);
}

void receive_large_file(int sockfd, const char *file_name)
{
    FILE *file = fopen(file_name, "wb");
    if (file == NULL)
    {
        perror("Error opening file");
        return;
    }

    char buffer[1024];
    ssize_t bytes_received;
    while ((bytes_received = recv(sockfd, buffer, sizeof(buffer), 0)) > 0)
    {
        fwrite(buffer, 1, bytes_received, file);
    }

    fclose(file);
}

// void run_program_chat(int isClient,int isServer,  ip, port, print_usage)
void run_program_chat(int isClient, int isServer, char *ip, char *port)
{
    if (isClient)
    {

        run_regular_chat_client(ip, port);
        printf("\n\033[1;33mYou: \033[0mEnter 'exit' to quit or select another usage:\n");
        print_usage();
        char *exitCmd = "exit\n";
        char *input = malloc(sizeof(char) * 100);
        fgets(input, 100, stdin);
        if (strcmp(input, exitCmd) == 0)
        {
            printf("\033[1;31mDisconnected from the server. Goodbye!\033[0m\n");
            free(input);
            exit(0);
        }
        free(input);
    }
    else if (isServer)
    {

        run_regular_chat_server(port);
    }
    // else if (isClient && isServer)
    print_usage();
}
