#include "partB.h"
#include <string.h>

void generate_file(char *filename, long size_in_bytes, int quiet)
{
    FILE *fp = fopen(filename, "w");
    if (fp == NULL)
    {
        printf("Error opening file '%s'\n", filename);
        return;
    }

    const int chunk_size = 1024 * 1024; // 1MB
    char buffer[chunk_size];
    int bytes_written = 0;

    while (bytes_written < size_in_bytes)
    {
        int bytes_to_write = chunk_size;
        if (bytes_written + bytes_to_write > size_in_bytes)
        {
            bytes_to_write = size_in_bytes - bytes_written;
        }
        fwrite(buffer, bytes_to_write, 1, fp);
        bytes_written += bytes_to_write;
    }

    fclose(fp);
    if (!quiet)
        printf("Generated file '%s' of size %ld bytes\n", filename, size_in_bytes);
}

uint32_t generate_checksum(char *filename, int quiet)
{
    FILE *fp = fopen(filename, "rb");
    if (fp == NULL)
    {
        printf("Error opening file '%s'\n", filename);
        return -1;
    }

    const int chunk_size = 1024 * 1024; // 1MB
    char buffer[chunk_size];
    uint32_t checksum = 0;

    while (!feof(fp))
    {
        size_t bytes_read = fread(buffer, 1, chunk_size, fp);
        for (size_t i = 0; i < bytes_read; i++)
        {
            checksum += (uint32_t)buffer[i];
        }
    }

    fclose(fp);
    if (!quiet)
        printf("Generated checksum for file '%s': 0x%08x\n", filename, checksum);
    return checksum;
}

int delete_file(char *filename, int quiet)
{
    int status = remove(filename);
    if (status != 0)
    {
        printf("Error deleting file '%s'\n", filename);
        return -1;
    }
    if (!quiet)
        printf("File '%s' deleted successfully\n", filename);
    return 0;
}

void print_time_diff(struct timeval *start, struct timeval *end)
{
    long seconds = end->tv_sec - start->tv_sec;
    long microseconds = end->tv_usec - start->tv_usec;
    if (microseconds < 0)
    {
        microseconds += 1000000;
        seconds--;
    }
    long milliseconds = seconds * 1000 + microseconds / 1000;
    printf("%ld\n", milliseconds);
};

int file_size(char *filename)
{
    struct stat st;
    stat(filename, &st);
    return st.st_size;
}
void initialize_variables(int *isClient, int *isServer, int *isTest, int *quiet, char **ip, char **port, char **type, char **param)
{
    *isClient = 0;
    *isServer = 0;
    *isTest = 0;
    *quiet = 0;
    *ip = NULL;
    *port = NULL;
    *type = NULL;
    *param = NULL;
}

void send_file(char *ip, char *port, char *filename, int domain, int type, int protocol, int quiet)
{
    if (!quiet)
        printf("Sending file '%s' to %s:%s\n", filename, ip, port);
    // Open File
    FILE *fp = fopen(filename, "rb");
    if (fp == NULL)
    {
        printf("ERROR opening file\n");
        exit(1);
    }
    int filesize = file_size(filename);

    // Create Socket
    int sockfd = socket(domain, type, protocol);
    if (sockfd < 0)
    {
        printf("ERROR opening socket\n");
        exit(1);
    }
    struct sockaddr_storage addr;
    socklen_t addr_len;

    if (domain == AF_INET6)
    {
        setIPv6Address(&addr, ip, port, &addr_len, sockfd);
    }
    else if (domain == AF_INET)
    {
        setIPv4Address(&addr, ip, port, &addr_len);
    }
    else if (domain == AF_UNIX)
    {
        setUnixAddress(&addr, port, &addr_len);
    }

    if (type == SOCK_STREAM || domain == AF_UNIX)
    {
        if (connect(sockfd, (struct sockaddr *)&addr, addr_len) < 0)
        {
            printf("ERROR connecting\n");
            exit(1);
        }
        if (!quiet)
            printf("Connected to %s:%s\n", ip, port);
    }

    // Send file
    char buffer[BUFFER_SIZE] = {0};
    int sent_bytes = 0;
    int bytes_read = 0;
    while (sent_bytes < filesize)
    {
        bytes_read = min(BUFFER_SIZE, filesize - sent_bytes); // Read at most BUFFER_SIZE bytes
        fread(buffer, 1, bytes_read, fp);
        int bytes_sent;
        if (type == SOCK_STREAM || domain == AF_UNIX)
        {
            bytes_sent = send(sockfd, buffer, bytes_read, 0);
        }
        else if (type == SOCK_DGRAM)
        {
            sleep(0.01); // delay to make sure packets are sent in order
            bytes_sent = sendto(sockfd, buffer, bytes_read, 0, (struct sockaddr *)&addr, sizeof(addr));
        }
        if (bytes_sent < 0)
        {
            printf("ERROR send() failed (FILE)\n");
            exit(1);
        }
        sent_bytes += bytes_sent;
        bzero(buffer, BUFFER_SIZE);
    }
    fclose(fp);
    if (!quiet)
        printf("File '%s' sent successfully\n", filename);
    close(sockfd);
}

int recive_file(char *port, int domain, int type, int protocol, int filesize, int quiet)
{
    if (!quiet)
        printf("Reciving file on port %s\n", port);
    // Create Socket
    int sockfd = socket(domain, type, protocol);
    if (sockfd < 0)
    {
        printf("ERROR opening socket\n");
        exit(1);
    }

    struct sockaddr_storage serveraddr, clientaddr;
    // Bind Socket

    socklen_t addr_size;

    if (domain == AF_INET6)
    { // ipv6
        int optval = 1;
        setsockopt(sockfd, IPPROTO_IPV6, IPV6_V6ONLY, &optval, sizeof(optval));
        setup_ipv6_server_address(&serveraddr, port);
        addr_size = sizeof(struct sockaddr_in6);
    }
    else if (domain == AF_INET)
    { // ipv4
        setup_ipv4_server_address(&serveraddr, port);
        addr_size = sizeof(struct sockaddr_in);
    }
    else if (domain == AF_UNIX)
    { // unix
        setup_unix_server_address(&serveraddr, port);
        addr_size = sizeof(struct sockaddr_un);
    }

    else
    {
        print_error_and_exit("ERROR invalid domain");
    }

    if (bind(sockfd, (struct sockaddr *)&serveraddr, addr_size) < 0)
    {
        print_error_and_exit("ERROR on binding");
    }

    struct pollfd fds[2];
    fds[0] = (struct pollfd){.fd = sockfd, .events = POLLIN};
    int newsockfd;
    if (type == SOCK_STREAM)
    {

        listen(sockfd, 1);
        if (!quiet)
            printf("Listening on port %s\n", port);

        newsockfd = accept(sockfd, (struct sockaddr *)&serveraddr, &addr_size);
        if (newsockfd < 0)
        {
            print_error_and_exit("ERROR on accept");
        }

        if (!quiet)
            printf("Accepted connection \n");

        fds[1] = (struct pollfd){.fd = newsockfd, .events = POLLIN};
    }

    // Recive File
    char buffer[BUFFER_SIZE] = {0};
    FILE *fp = fopen("recived.txt", "wb");
    int size = 0;
    while (size < filesize)
    {
        int pollRes = poll(fds, 2, 2000);
        if (pollRes == 0)
        {
            if (!quiet)
                printf("Timeout\n");
            break;
        }
        if (pollRes < 0)
        {
            print_error_and_exit("ERROR: poll failed");
        }

        int recived;
        if (type == SOCK_STREAM)
        {
            recived = recv(newsockfd, buffer, BUFFER_SIZE, 0);
            if (recived < 0)
            {
                print_error_and_exit("ERROR: recv failed");
            }
        }
        else if (type == SOCK_DGRAM)
        {
            recived = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&clientaddr, &addr_size);
            if (recived < 0)
            {
                print_error_and_exit("ERROR: recvfrom failed");
            }

            if (recived == 0)
            {
                break;
            }
            size += recived;

            // Write to file
            fwrite(buffer, recived, 1, fp);
            bzero(buffer, BUFFER_SIZE);
        }
        fclose(fp);
        if (type == SOCK_STREAM)
            close(newsockfd);
        close(sockfd);
        if (domain == AF_UNIX)
        {
            unlink(port);
        }
        return size;
    }
}
void copy_file_to_shm_mmap(char *filenameFrom, char *sharedFilename, int quiet)
{
    if (!quiet)
        printf("Copying file '%s' to shared memory '%s'\n", filenameFrom, sharedFilename);
    int fdFrom = open(filenameFrom, O_RDONLY);
    if (fdFrom < 0)
    {
        print_error_and_exit("ERROR opening file\n");
    }
    int fileSize = file_size(filenameFrom);

    int shm_fd = shm_open(sharedFilename, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (shm_fd < 0)
    {
        print_error_and_exit("ERROR creating shared memory\n");
    }
    if (ftruncate(shm_fd, fileSize) < 0)
    {
        print_error_and_exit("ERROR truncating shared memory\n");
    }

    char *shm_ptr = mmap(NULL, fileSize, PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED)
    {
        print_error_and_exit("ERROR mapping shared memory\n");
    }
    char *file_ptr = mmap(NULL, fileSize, PROT_READ, MAP_SHARED, fdFrom, 0);
    if (file_ptr == MAP_FAILED)
    {
        print_error_and_exit("ERROR mapping file from\n");
    }

    memcpy(shm_ptr, file_ptr, fileSize);

    if (munmap(shm_ptr, fileSize) < 0 || munmap(file_ptr, fileSize) < 0)
    {
        print_error_and_exit("ERROR unmapping\n");
    }
    close(fdFrom);
    close(shm_fd);
}

void copy_file_from_shm_mmap(char *filenameTo, char *sharedFilename, int fileSize, int quiet)
{
    if (!quiet)
        printf("Copying file from shared memory '%s' to '%s'\n", sharedFilename, filenameTo);
    int fdTo = open(filenameTo, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fdTo < 0)
    {
        print_error_and_exit("ERROR opening file\n");
    }
    if (ftruncate(fdTo, fileSize) < 0)
    {
        print_error_and_exit("ERROR truncating file\n");
    }
    int shm_fd = shm_open(sharedFilename, O_RDWR, S_IRUSR | S_IWUSR);
    if (shm_fd < 0)
    {
        printf("ERROR opening shared memory\n");
        exit(1);
    }

    char *shm_ptr = mmap(NULL, fileSize, PROT_READ, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED)
    {
        printf("ERROR mapping shared memory\n");
        exit(1);
    }
    char *file_ptr = mmap(NULL, fileSize, PROT_WRITE, MAP_SHARED, fdTo, 0);
    if (file_ptr == MAP_FAILED)
    {
        print_error_and_exit("ERROR mapping file\n");
    }
    memcpy(file_ptr, shm_ptr, fileSize);

    if (munmap(shm_ptr, fileSize) < 0 || munmap(file_ptr, fileSize) < 0)
    {
        printf("ERROR unmapping shared memory\n");
        exit(1);
    }

    // Unlink shared memory to free resources
    if (shm_unlink(sharedFilename) < 0)
    {
        print_error_and_exit("ERROR unlinking shared memory\n");
    }
    

    close(fdTo);
    close(shm_fd);
}

void send_file_fifo(char *filenameFrom, char *fifoName, int quiet)
{
    if (!quiet)
        printf("Sending file '%s' to fifo '%s'\n", filenameFrom, fifoName);
    char buffer[BUFFER_SIZE] = {0};
    int fdFrom = open(filenameFrom, O_RDONLY);
    if (fdFrom < 0)
    {
        printf("ERROR opening file from\n");
        exit(1);
    }

    // Create fifo
    if (mkfifo(fifoName, 0666) < 0 && errno != EEXIST)
    {
        printf("ERROR creating fifo\n");
        exit(1);
    }

    int fdFifo = open(fifoName, O_WRONLY);
    if (fdFifo < 0)
    {
        printf("ERROR opening fifo\n");
        exit(1);
    }

    ssize_t read_res, write_res;
    while ((read_res = read(fdFrom, buffer, BUFFER_SIZE)) > 0)
    {
        write_res = write(fdFifo, buffer, read_res);
        if (write_res != read_res)
        {
            printf("ERROR writing to fifo\n");
            exit(1);
        }
    }

    close(fdFrom);
    close(fdFifo);
}

void recive_file_fifo(char *filenameTo, char *fifoName, int quiet)
{
    if (!quiet)
        printf("Reciving file from fifo '%s' to '%s'\n", fifoName, filenameTo);
    char buffer[BUFFER_SIZE] = {0};
    int fdTo = open(filenameTo, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fdTo < 0)
    {
        printf("ERROR opening file to\n");
        exit(1);
    }

    int fdFifo = open(fifoName, O_RDONLY);
    if (fdFifo < 0)
    {
        printf("ERROR opening fifo\n");
        exit(1);
    }

    ssize_t read_res, write_res;
    while ((read_res = read(fdFifo, buffer, BUFFER_SIZE)) > 0)
    {
        write_res = write(fdTo, buffer, read_res);
        if (write_res != read_res)
        {
            printf("ERROR writing to file\n");
            exit(1);
        }
    }

    // Unlink fifo to free resources
    if (unlink(fifoName) < 0)
    {
        printf("ERROR unlinking fifo\n");
        exit(1);
    }

    close(fdTo);
    close(fdFifo);
}

int get_file_size(char *filename)
{
    struct stat statFrom; // Get file size
    if (stat(filename, &statFrom) < 0)
    {
        printf("ERROR stat\n");
        exit(1);
    }
    return statFrom.st_size;
}

int min(int a, int b)
{
    if (a < b)
    {
        return a;
    }
    return b;
}

int run_test_client(int isTest, int isClient, char *type, char *param, int *ipv4, int *ipv6, int *uds, int *isMmap, int *isPipe, int *isTCP, int *udp, int *dgram, int *stream, char **filename)
{
    if (isTest && isClient)
    {
        if (!type || !param)
        {
            printf("No type or param after -p\n");
            return 1;
        }

        if (strcmp(type, "ipv4") == 0)
        {
            *ipv4 = 1;
            printf("IPv4\n (run client test)");
        }
        else if (strcmp(type, "ipv6") == 0)
        {
            *ipv6 = 1;
        }
        else if (strcmp(type, "uds") == 0)
        {
            *uds = 1;
        }
        else if (strcmp(type, "mmap") == 0)
        {
            *isMmap = 1;
        }
        else if (strcmp(type, "pipe") == 0)
        {
            *isPipe = 1;
        }
        else
        {
            printf("Wrong type after -p (ipv4, ipv6, uds, mmap, pipe)\n");
            return 1;
        }

        if (strcmp(param, "isTCP") == 0)
        {
            *isTCP = 1;
        }
        else if (strcmp(param, "udp") == 0)
        {
            *udp = 1;
        }
        else if (strcmp(param, "dgram") == 0)
        {
            *dgram = 1;
        }
        else if (strcmp(param, "stream") == 0)
        {
            *stream = 1;
        }
        else
        {
            *filename = param;
        }
    }

    return 0;
}
