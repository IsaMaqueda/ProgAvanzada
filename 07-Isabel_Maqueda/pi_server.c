/*
    Server program to compute the value of PI
    This program calls the library function 'get_pi'
    It will create child processes to attend requests
    It receives connections using sockets

    Isabel Maqueda Rolon
    A01652906
    24/10/2019
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <sys/poll.h>
#include <signal.h>

// Sockets libraries
#include <netdb.h>
#include <arpa/inet.h>

// Custom libraries
#include "get_pi.h"
#include "sockets.h"
#include "fatal_error.h"

#define BUFFER_SIZE 1024
#define MAX_QUEUE 5

///// FUNCTION DECLARATIONS
void usage(char *program);
void waitForConnections(int server_fd);
void attendRequest(int client_fd);
void setupHandlers();
void onInterrupt(int signal);
int interrupted = 0;

//BL0CK SIGNALS
sigset_t setupMask();
void unsetMask(sigset_t old_set);

///// MAIN FUNCTION
int main(int argc, char *argv[])
{
    int server_fd;

    printf("\n=== SERVER FOR COMPUTING THE VALUE OF pi ===\n");

    // Check the correct arguments
    if (argc != 2)
    {
        usage(argv[0]);
    }
    setupHandlers();
    sigset_t old;

    old = setupMask();

    // Show the IPs assigned to this computer
    printLocalIPs();

    // Start the server
    server_fd = startServer(argv[1]);
    // Listen for connections from the clients
    waitForConnections(server_fd);
    // Close the socket
    close(server_fd);
    unsetMask(old);

    return 0;
}

///// FUNCTION DEFINITIONS

/*
    Explanation to the user of the parameters required to run the program
*/
void usage(char *program)
{
    printf("Usage:\n");
    printf("\t%s {port_number}\n", program);
    exit(EXIT_FAILURE);
}

/*
    Main loop to wait for incomming connections
*/
void waitForConnections(int server_fd)
{
    struct sockaddr_in client_address;
    socklen_t client_address_size;
    char client_presentation[INET_ADDRSTRLEN];
    int connection_fd;
    pid_t new_pid;
    // Variables for polling
    struct pollfd poll_fd[1];
    int poll_response;
    int timeout = 1000; // Timeout of one second

    // Prepare for the poll
    poll_fd[0].fd = server_fd;
    poll_fd[0].events = POLLIN;

    while (!interrupted)
    {
        ///// ACCEPT
        // Receive incomming connections
        // Get the size of the structure where the address of the client will be stored
        client_address_size = sizeof client_address;

        while (1)
        {
            poll_response = poll(poll_fd, 1, timeout);
            if (poll_response == 0) // Nothing to receive
            {
                printf(".");
                fflush(stdout);
            }
            else if (poll_response == -1)
            {
                perror("poll");
                // Exit the inner loop
                break;
            }
            else
            {
                printf("A client wants to connect\n");
                if (poll_fd[0].revents & POLLIN)
                {
                    // Get the file descriptor to talk with the client
                    connection_fd = accept(server_fd, (struct sockaddr *)&client_address,
                                           &client_address_size);
                    break;
                }
            }
        }

        if (!interrupted)
        {
            // Fork a child process to deal with the new client
            new_pid = fork();
            if (new_pid > 0) // Parent
            {
                close(connection_fd);
            }
            else if (new_pid == 0) // Child
            {
                close(server_fd);

                // Identify the client
                // Get the ip address from the structure filled by accept
                inet_ntop(client_address.sin_family, &client_address.sin_addr, client_presentation, INET_ADDRSTRLEN);
                printf("Received a connection from %s : %d\n", client_presentation, client_address.sin_port);

                // Establish the communication
                attendRequest(connection_fd);
                close(connection_fd);
                // Finish the child process
                exit(EXIT_SUCCESS);
            }
        }
    }
}

/*
    Hear the request from the client and send an answer
*/
void attendRequest(int client_fd)
{
    char buffer[BUFFER_SIZE];
    unsigned long int iterations;
    double result;
    int chars_read;

    // Variables for polling
    struct pollfd poll_fd[1];
    int poll_response;
    int timeout = 1000; // Timeout of one second

    // Prepare for the poll
    poll_fd[0].fd = client_fd;
    poll_fd[0].events = POLLIN;

    // RECV
    // Receive the request
    chars_read = receiveMessage(client_fd, buffer, BUFFER_SIZE);
    sscanf(buffer, "%lu", &iterations);

    printf(" > Got request from client with iterations=%lu\n", iterations);

    int sign = -1;
    unsigned long int divisor = 3;
    unsigned long int counter = 0;

    while (1)
    {
        result += sign * (4.0 / divisor);
        sign *= -1;
        divisor += 2;
        counter++;

        poll_response = poll(poll_fd, 1, timeout);
        if (poll_response == 0) // Nothing to receive
        {
            printf(".");
            fflush(stdout);
        }
        else if (poll_response == -1)
        {
            perror("poll");
            // Exit the inner loop
            break;
        }
        else
        {
            printf("Connection interrupted");
            if (poll_fd[0].revents & POLLIN)
            {
                //Recieves the message
                chars_read = receiveMessage(client_fd, buffer, BUFFER_SIZE);
                break;
            }
        }
        if (counter >= iterations)
        {
            break;
        }
        if (interrupted)
        {
            break;
        }
    }

    //chars_read = receiveMessage(client_fd, buffer, BUFFER_SIZE);

    printf(" > Sending PI=%.20lf\n", result);
    //printf("%ld, %.20lf \n ", counter,result);
    // Prepare the response to the client
    sprintf(buffer, "The value for PI is : %.20lf for %ld iterations", result, counter);
    // SEND
    // Send the response
    send(client_fd, buffer, strlen(buffer) + 1, 0);
}

// Define signal handlers
void setupHandlers()
{
    struct sigaction new_action;
    new_action.sa_handler = onInterrupt;
    sigaction(SIGINT, &new_action, NULL);
}

// Handler for SIGINT
void onInterrupt(int signal)
{
    interrupted = 1;
}

// Modify the signal mask
sigset_t setupMask()
{
    sigset_t new_set;
    sigset_t old_set;

    //Block all signals
    sigfillset(&new_set);

    //Remove the SIGNIT
    sigdelset(&new_set, SIGINT);

    // Apply the set to the program
    if (sigprocmask(SIG_BLOCK, &new_set, &old_set) == -1)
    {
        perror("sigprocmask");
    }

    return old_set;
}

// Reset the blocking mask to the previous one
void unsetMask(sigset_t old_set)
{
    sigprocmask(SIG_SETMASK, &old_set, NULL);
}