/*
    Client program to access the accounts in the bank
    This program connects to the server using sockets

    Isabel Maqueda Rolon
    A01652906
    14/11/2019
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
// Thread library
#include <pthread.h>
// Sockets libraries
#include <netdb.h>
#include <arpa/inet.h>
// Custom libraries
#include "bank_codes.h"
#include "sockets.h"
#include "fatal_error.h"

#define BUFFER_SIZE 1024

#define NUM_THREADS 6
#define NUM_DEPOSITS 100000
#define DEPOSIT_AMOUNT 5.0
#define ACCOUNT_1 1
#define ACCOUNT_2 2

typedef struct server_info_structure {
    char * address;
    char * port;
    operation_t operation;
    int from;
    int to;
} server_info_t;

///// FUNCTION DECLARATIONS
void usage(char * program);
void * clientThread(void * arg);
void bankOperations(int connection_fd, server_info_t * server_info);
void makeOperation(int connection_fd, char * request, char * response);
void printResult(char * response, int account);

///// MAIN FUNCTION
int main(int argc, char * argv[])
{
    server_info_t server_info[NUM_THREADS];
    pthread_t tid[NUM_THREADS];
    int thread_status;

    printf("\n=== BANK CLIENT PROGRAM ===\n");

    // Check the correct arguments
    if (argc != 3)
    {
        usage(argv[0]);
    }

    printf("--- Stage 1: deposits ---\n");

    // Start the threads
    for (int i=0; i<NUM_THREADS; i++)
    {
        // Fill in the information for each thread
        server_info[i].address = argv[1];
        server_info[i].port = argv[2];
        server_info[i].operation = DEPOSIT;

        if (i % 2 == 0)
        {
            server_info[i].from = ACCOUNT_1;
            server_info[i].to = ACCOUNT_2;
        }
        else
        {
            server_info[i].from = ACCOUNT_2;
            server_info[i].to = ACCOUNT_1;
        }

        thread_status = pthread_create(&tid[i], NULL, &clientThread, (void *)&server_info[i]);
        // Check for errors
        if (thread_status)
        {
            fprintf(stderr, "ERROR: pthread_create %d\n", thread_status);
            exit(EXIT_FAILURE);
        }

        printf("Created thread %d with ID %lu\n", i, tid[i]);
    }

    // Wait for the threads to finish
    for(int i=0; i<NUM_THREADS; i++)
    {
        pthread_join(tid[i], NULL);
        printf("Thread %d finished\n", i);
    }

    printf("--- Stage 2: mixed ---\n");

    // Start the threads
    for (int i=0; i<NUM_THREADS; i++)
    {
        // Set different operations for the threads
        if (i < 2)
        {
            server_info[i].operation = WITHDRAW;
        }
        else if (i < 4)
        {
            server_info[i].operation = TRANSFER;
        }
        else
        {
            server_info[i].operation = DEPOSIT;
        }

        thread_status = pthread_create(&tid[i], NULL, &clientThread, (void *)&server_info[i]);
        // Check for errors
        if (thread_status)
        {
            fprintf(stderr, "ERROR: pthread_create %d\n", thread_status);
            exit(EXIT_FAILURE);
        }

        printf("Created thread %d with ID %lu\n", i, tid[i]);
    }

    // Finish the threads
    for(int i=0; i<NUM_THREADS; i++)
    {
        pthread_join(tid[i], NULL);
        printf("Thread %d finished\n", i);
    }

    return 0;
}

///// FUNCTION DEFINITIONS

/*
    Thread function to make a connection to the bank server
    Receives a pointer to a structure with the server address and port
*/
void * clientThread(void * arg)
{
    server_info_t * server_info = (server_info_t *) arg;
    int connection_fd;

    // Start the server
    connection_fd = connectSocket(server_info->address, server_info->port);
	// Use the bank operations available
    bankOperations(connection_fd, server_info);
    // Close the socket
    close(connection_fd);

    pthread_exit(NULL);
}

/*
    Explanation to the user of the parameters required to run the program
*/
void usage(char * program)
{
    printf("Usage:\n");
    printf("\t%s {server_address} {port_number}\n", program);
    exit(EXIT_FAILURE);
}

/*
    Automatic operation, depending on the parameters
*/
void bankOperations(int connection_fd, server_info_t * server_info)
{
    char request[BUFFER_SIZE];
    char response[BUFFER_SIZE];

    printf("Requesting: %d %d %d %f\n", server_info->operation, server_info->from, server_info->to, DEPOSIT_AMOUNT);

    // Loop to make lots of deposits in to the same account
    for (int i=0; i<NUM_DEPOSITS; i++)
    {
        // Prepare the message to the server
        sprintf(request, "%d %d %d %f", server_info->operation, server_info->from, server_info->to, DEPOSIT_AMOUNT);
        // Send the request and wait for the response
        makeOperation(connection_fd, request, response);
    }

    // Check the final balance after the deposits
    // Prepare the message to the server
    sprintf(request, "%d %d %d %f", CHECK, server_info->from, 0, 0.0);
    // Send the request and wait for the response
    makeOperation(connection_fd, request, response);
    // Show the results
    printResult(response, server_info->from);

    // Disconnect from the server
    // Prepare the message to the server
    sprintf(request, "%d %d %d %f", EXIT, 0, 0, 0.0);
    // Send the request and wait for the response
    makeOperation(connection_fd, request, response);
    // Show the results
    printResult(response, 0);
}

void makeOperation(int connection_fd, char * request, char * response)
{
    // SEND
    // Send the request
    sendString(connection_fd, request, strlen(request)+1);
    // RECV
    // Receive the response
    if ( !recvString(connection_fd, response, BUFFER_SIZE) )
    {
        printf("Server closed the connection\n");
        exit(EXIT_FAILURE);
    }
}

void printResult(char * response, int account)
{
    float balance;
    int status;

    // Extract the data
    sscanf(response, "%d %f", &status, &balance);

    // Print the result
    switch (status)
    {
        case OK:
            printf("\tResponse from thread %ld\n", pthread_self());
            printf("\tThe balance in account %d is %.2f\n", account, balance);
            break;
        case INSUFFICIENT:
            printf("\tInsufficient funds for the transaction selected\n");
            break;
        case NO_ACCOUNT:
            printf("\tInvalid acount number entered\n");
            break;
        case BYE:
            printf("\tThanks for connecting to the bank. Good bye!\n");
            break;
        case ERROR: default:
            printf("\tInvalid operation. Try again\n");
            break;
    }

}
