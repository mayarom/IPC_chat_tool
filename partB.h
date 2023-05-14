#ifndef PARTB_H
#define PARTB_H

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



#define BUFFER_SIZE 32768 // 32KB udp cant send more

void generate_file(char *filename, long size_in_bytes, int quite);
uint32_t generate_checksum(char *filename, int quite);
int delete_file(char *filename, int quite);
void print_time_diff(struct timeval *start, struct timeval *end);
void send_file(char *ip, char *port, char *filename, int domain, int type, int protocol, int quite);
int recive_file(char *port, int domain, int type, int protocol, int filesize, int quiet);
int min(int a, int b);
void initialize_variables(int *isClient, int *isServer, int *isTest, int *quiet, char **ip, char **port, char **type, char **param);
int file_size(char *filename);
void copy_file_to_shm_mmap(char *filenameFrom, char *sharedFilename, int quiet);
void copy_file_from_shm_mmap(char *filenameTo, char *sharedFilename, int fileSize, int quiet);
void recive_file_fifo(char *filenameTo, char *fifoName, int quiet);
void send_file_fifo(char *filenameFrom, char *fifoName, int quiet);
int run_test_client(int isTest, int isClient, char *type, char *param, int *ipv4, int *ipv6, int *uds, int *isMmap, int *isPipe, int *isTCP, int *udp, int *dgram, int *stream, char **filename);

#endif