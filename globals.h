#ifndef GLOBALS_H
#define GLOBALS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include "partB.h"
#include "stnc.h"

#define BUFFER_SIZE 32768 

extern int udp, uds, dgram, stream, isMmap, isPipe, deleteFile, quiet;
extern char *filename, *ip, *port, *type, *param;
extern int isClient, isServer, isTest, ipv4, ipv6, isTCP, isUDP, isUDS, isDgram, isStream, isMmapEnabled, isPipeEnabled, isDeleteFile, isQuietMode;
extern char *fileName, *ipAddress, *portNumber, *connectionType, *parameter;

#endif // GLOBALS_H
