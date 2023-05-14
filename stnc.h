#ifndef STNC_H
#define STNC_H

#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netdb.h>
#include <sys/un.h>
#include <sys/time.h>
#include <poll.h>
#include <arpa/inet.h>
#include "globals.h"
#include "common.h"
#include "partB.h"
#include "server.h"
#include "client.h"

// Buffer size for sending and receiving messages
#define BUFFER_SIZE_MESSAGE 1024

// Function to run the isClient
void client_main_func(char *ip, char *port);

// Function to run the isServer
void server_main_func(char *port);

// Function to print the isServer title
void printServerTitle();

// Function to print the isClient title
void printClientTitle();

// Function to print the usage of the program
void print_usage();

// Function to print an error message and exit the program
void print_error_and_exit(char *msg);

// Function to find the size of a file
int find_file_size(char *filename);

// Function to check if the user input is a quit command
int is_quit(char *messageBuffer);

// Function to create a pollfd struct for polling
void create_pollfd(int sockfd, struct pollfd *fds);

// Function to send a message and return the number of bytes sent
int byte_sent(int sockfd, char *messageBuffer, int messageSize);

// Add this line to stnc.h
void parse_arguments(int argc, char *argv[], int *isClient, int *isServer, int *isTest, int *quiet, char **ip, char **port, char **type, char **param);

void run_uds_test(int uds, int dgram, int stream, int sockfd, char *new_port, int quiet);

#endif
