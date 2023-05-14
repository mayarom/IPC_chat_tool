#ifndef COMMON_H
#define COMMON_H

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "partB.h"
#include "stnc.h"
#include "globals.h"
#include "common.h"

// Function to set an IPv6 address
void setIPv6Address(struct sockaddr_storage *addr, const char *ip, const char *port, socklen_t *addr_len, int sockfd);

// Function to set an IPv4 address
void setIPv4Address(struct sockaddr_storage *addr, const char *ip, const char *port, socklen_t *addr_len);

// Function to set a Unix address
void setUnixAddress(struct sockaddr_storage *addr, const char *port, socklen_t *addr_len);

// Function to set up IPv6 server address
void setup_ipv6_server_address(struct sockaddr_storage *serveraddr, const char *port);

// Function to set up IPv4 server address
void setup_ipv4_server_address(struct sockaddr_storage *serveraddr, const char *port);

// Function to set up Unix domain server address
void setup_unix_server_address(struct sockaddr_storage *serveraddr, const char *path);

#endif // COMMON_H
