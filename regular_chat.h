#ifndef REGULAR_CHAT_H
#define REGULAR_CHAT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include "globals.h"

#define BUFFER_SIZE 1024

void run_regular_chat_server(char *port);
void run_regular_chat_client(char *ip, char *port);


#endif