/*
    Program for a simple chat server
    Can only connect with one client at a time
    Isabel Maqueda Rolon
    A01652906
    07/10/19
*/

#ifndef SOCKETS_H
#define SOCKETS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

// Include libraries for sockets
#include <netdb.h>
#include <arpa/inet.h>
#include <ifaddrs.h>


#define SERVICE_PORT 8642
#define MAX_QUEUE 5
#define BUFFER_SIZE 1024

void printLocalIPs();
int startServer(char * port);
int connectToServer(char * address, char * port);
int receiveMessage(int connection_fd, char * buffer, int size);

#endif
