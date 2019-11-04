/*
    Program for a simple chat server
    Can only connect with one client at a time
    Isabel Maqueda Rolon 
    A01652906
    24/10/2019
*/

#include "sockets.h"

/*
	Show the local IP addresses, to allow testing
	Based on code from:
		https://stackoverflow.com/questions/20800319/how-do-i-get-my-ip-address-in-c-on-linux
*/
void printLocalIPs()
{
	struct ifaddrs * addrs;
	struct ifaddrs * tmp;

    // Get the list of IP addresses used by this machine
	getifaddrs(&addrs);
	tmp = addrs;

    printf("Server IP addresses:\n");

	while (tmp) 
	{
		if (tmp->ifa_addr && tmp->ifa_addr->sa_family == AF_INET)
		{
            // Get the address structure casting as IPv4
			struct sockaddr_in *pAddr = (struct sockaddr_in *)tmp->ifa_addr;
            // Print the ip address of the local machine
			printf("%s: %s\n", tmp->ifa_name, inet_ntoa(pAddr->sin_addr));
		}
        // Advance in the linked list
		tmp = tmp->ifa_next;
	}

	freeifaddrs(addrs);
}

// Initialize the server to be ready for connections
int startServer(char * port)
{
    struct addrinfo hints;
    struct addrinfo * server_info = NULL;
    int server_fd;
    
    // Prepare the information to determine the local address
    // Clear the structure
    bzero(&hints, sizeof hints);
    // Let the IP address be found automatically
    hints.ai_flags = AI_PASSIVE;
    // Use IPv4 address
    hints.ai_family = AF_INET;
    // Type of socket
    hints.ai_socktype = SOCK_STREAM;

    ///// GETADDRINFO
    // Get the actual address of this computer
    if (getaddrinfo(NULL, port, &hints, &server_info) == -1)
    {
        perror("getaddrinfo");
        exit(EXIT_FAILURE);
    }

    ///// SOCKET
    // Use the information obtained by getaddrinfo
    server_fd = socket(server_info->ai_family, server_info->ai_socktype,
                        server_info->ai_protocol);
    if (server_fd == -1)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    
    // Allow using the same port immediately, even if the previous server did not close correctly
    int reuse = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof reuse) == -1)
    {
        perror("setsockopt");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    ///// BIND
    // Connect with the desired port
    if (bind(server_fd, server_info->ai_addr, server_info->ai_addrlen) == -1)
    {
        perror("bind");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Release the memory of the list of addrinfo
    freeaddrinfo(server_info);
    
    ///// LISTEN
    // Prepare for connections
    if (listen(server_fd, MAX_QUEUE) == -1)
    {
        perror("listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server ready and waiting!\n");

    return server_fd;
}

// Establish a connection with the server indicated by the parameters
int connectToServer(char * address, char * port)
{
    struct addrinfo hints;
    struct addrinfo * server_info = NULL;
    int client_fd;

    // Prepare the information to determine the local address
    // Clear the structure
    bzero(&hints, sizeof hints);
    // Let the IP address be found automatically
    hints.ai_flags = AI_PASSIVE;
    // Use IPv4 address
    hints.ai_family = AF_INET;
    // Type of socket
    hints.ai_socktype = SOCK_STREAM;

    ///// GETADDRINFO
    // Get the actual address of this computer
    if (getaddrinfo(address, port, &hints, &server_info) == -1)
    {
        perror("getaddrinfo");
        exit(EXIT_FAILURE);
    }

    ///// SOCKET
    // Use the information obtained by getaddrinfo
    client_fd = socket(server_info->ai_family, server_info->ai_socktype,
                        server_info->ai_protocol);
    if (client_fd == -1)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    ///// CONNECT
    // Connect to the server
    if ( connect(client_fd, server_info->ai_addr, server_info->ai_addrlen) == -1)
    {
        perror("accept");
        close(client_fd);
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(server_info);

    return client_fd;
}

int receiveMessage(int connection_fd, char * buffer, int size)
{
    int chars_read;

    // Clear the buffer
    bzero(buffer, BUFFER_SIZE);
    // Get an incomming message
    chars_read = recv(connection_fd, buffer, BUFFER_SIZE, 0);
    // Error when receiving data
    if (chars_read == -1)
    {
        perror("recv");
        close(connection_fd);
        return -1;
    }
    else if (chars_read == 0)
    {
        printf("Connection closed by client\n");
        close(connection_fd);
        return 0;
    }

    return chars_read;
}