#include "common.h"

void setIPv6Address(struct sockaddr_storage *addr, const char *ip, const char *port, socklen_t *addr_len, int sockfd)
{
    int optval = 1;
    setsockopt(sockfd, IPPROTO_IPV6, IPV6_V6ONLY, &optval, sizeof(optval));
    struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)addr;
    memset((char *)addr6, 0, sizeof(*addr6));
    addr6->sin6_family = AF_INET6;
    addr6->sin6_port = htons(atoi(port));
    inet_pton(AF_INET6, ip, &addr6->sin6_addr);

    *addr_len = sizeof(*addr6);
}

void setIPv4Address(struct sockaddr_storage *addr, const char *ip, const char *port, socklen_t *addr_len)
{
    struct sockaddr_in *addr4 = (struct sockaddr_in *)addr;
    memset((char *)addr4, 0, sizeof(*addr4));
    addr4->sin_family = AF_INET;
    addr4->sin_port = htons(atoi(port));
    inet_pton(AF_INET, ip, &addr4->sin_addr);

    *addr_len = sizeof(*addr4);
}

void setUnixAddress(struct sockaddr_storage *addr, const char *port, socklen_t *addr_len)
{
    struct sockaddr_un *addr_un = (struct sockaddr_un *)addr;
    memset((char *)addr_un, 0, sizeof(*addr_un));
    addr_un->sun_family = AF_UNIX;
    strcpy(addr_un->sun_path, port); // port must be socket path

    *addr_len = sizeof(*addr_un);
}
// Function to set up IPv6 server address
void setup_ipv6_server_address(struct sockaddr_storage *serveraddr, const char *port)
{
    struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)serveraddr;
    memset((char *)addr6, 0, sizeof(*addr6));
    addr6->sin6_family = AF_INET6;
    addr6->sin6_port = htons(atoi(port));
    addr6->sin6_addr = in6addr_any;
}

// Function to set up IPv4 server address
void setup_ipv4_server_address(struct sockaddr_storage *serveraddr, const char *port)
{
    struct sockaddr_in *addr4 = (struct sockaddr_in *)serveraddr;
    memset((char *)addr4, 0, sizeof(*addr4));
    addr4->sin_family = AF_INET;
    addr4->sin_port = htons(atoi(port));
    addr4->sin_addr.s_addr = INADDR_ANY;
}

// Function to set up Unix domain server address
void setup_unix_server_address(struct sockaddr_storage *serveraddr, const char *path)
{
    struct sockaddr_un *addr_un = (struct sockaddr_un *)serveraddr;
    memset((char *)addr_un, 0, sizeof(*addr_un));
    addr_un->sun_family = AF_UNIX;
    strncpy(addr_un->sun_path, path, sizeof(addr_un->sun_path) - 1);
}
