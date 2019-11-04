/*
    Client program to get the value of PI
    This program connects to the server using sockets

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
#include "sockets.h"
#include "fatal_error.h"

#define BUFFER_SIZE 1024


///// FUNCTION DECLARATIONS
void usage(char * program);
void requestPI(int connection_fd);

//BL0CK SIGNALS
sigset_t setupMask();
void unsetMask(sigset_t old_set);

//HANDLE SIGNALS
void setupHandlers();
void onInterrupt(int signal);
int interrupted = 0;



///// MAIN FUNCTION
int main(int argc, char * argv[])
{
    int connection_fd;
    sigset_t mask;

    printf("\n=== CLIENT FOR COMPUTING THE VALUE OF pi ===\n");

    // Check the correct arguments
    if (argc != 3)
    {
        usage(argv[0]);
    }

    setupHandlers();
    
    
    mask = setupMask();

    // Start the server
    connection_fd = connectToServer(argv[1], argv[2]);
	// Listen for connections from the clients
    requestPI(connection_fd);


    // Close the socket
    //unsetMask(mask);

    close(connection_fd);
    unsetMask(mask);

    return 0;
}

///// FUNCTION DEFINITIONS

/*
    Explanation to the user of the parameters required to run the program
*/
void usage(char * program)
{
    printf("Usage:\n");
    printf("\t%s {server_address} {port_number}\n", program);
    exit(EXIT_FAILURE);
}


void requestPI(int connection_fd)
{
    char buffer[BUFFER_SIZE];
    unsigned long int iterations;
    //double iterations;
    int chars_read;


    // Variables for polling
    struct pollfd poll_fd[1];
    int poll_response;
    int timeout = 1000;     // Timeout of one second

    // Prepare for the poll
    poll_fd[0].fd = connection_fd;
    poll_fd[0].events = POLLIN;

    printf("Enter the number of iterations for PI: ");
    scanf("%lu", &iterations);

    // Prepare the response to the client
    sprintf(buffer, "%lu", iterations);
    // SEND
    // Send the response
    send(connection_fd, buffer, strlen(buffer)+1, 0);

        while(1)
        {
            poll_response = poll(poll_fd, 1, timeout);
            if (poll_response == 0)     // Nothing to receive
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
                if (poll_fd[0].revents & POLLIN)
                {
                    chars_read = receiveMessage(connection_fd, buffer, BUFFER_SIZE);
                    printf("%s \n", buffer);
                    break;
                }
            }
            if(interrupted)
            {
                break;
            }
        }

        sprintf(buffer,"Conection interrupted");
        send(connection_fd, buffer, strlen(buffer)+1, 0);
        
        chars_read = receiveMessage(connection_fd, buffer, BUFFER_SIZE);
        printf("%s \n", buffer);
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
    sigdelset(&new_set,SIGINT);

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