/*
    Program for a simple bank server
    It uses sockets and threads
    The server will process simple transactions on a limited number of accounts

    Isabel Maqueda Rolon 
    A01652906
    11/14/2019
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
// Signals library
#include <errno.h>
#include <signal.h>
// Sockets libraries
#include <netdb.h>
#include <sys/poll.h>
// Posix threads library
#include <pthread.h>

// Custom libraries
#include "bank_codes.h"
#include "sockets.h"
#include "fatal_error.h"

#define MAX_ACCOUNTS 5
#define BUFFER_SIZE 1024
#define MAX_QUEUE 5

///// Structure definitions

// Data for a single bank account
typedef struct account_struct {
    int id;
    int pin;
    float balance;
} account_t;

// Data for the bank operations
typedef struct bank_struct {
    // Store the total number of operations performed
    int total_transactions;
    // An array of the accounts
    account_t * account_array;
} bank_t;

// Structure for the mutexes to keep the data consistent
typedef struct locks_struct {
    // Mutex for the number of transactions variable
    pthread_mutex_t transactions_mutex;
    // Mutex array for the operations on the accounts
    pthread_mutex_t * account_mutex;
} locks_t;

// Data that will be sent to each structure
typedef struct data_struct {
    // The file descriptor for the socket
    int connection_fd;
    // A pointer to a bank data structure
    bank_t * bank_data;
    // A pointer to a locks structure
    locks_t * data_locks;
} thread_data_t;


// Global variables for signal handlers
int interrupt_exit = 0;


///// FUNCTION DECLARATIONS
void usage(char * program);
void setupHandlers();
void onInterrupt();
void initBank(bank_t * bank_data, locks_t * data_locks);
void readBankFile(bank_t * bank_data);
void writeBankFile(bank_t * bank_data);
void waitForConnections(int server_fd, bank_t * bank_data, locks_t * data_locks);
void * attentionThread(void * arg);
void closeBank(bank_t * bank_data, locks_t * data_locks);
int checkValidAccount(int account);

float check(int account, thread_data_t * connection_fd, response_t * response);
float deposit(int account, float amount, thread_data_t * connection_fd, response_t * response);
float withdraw(int account, float amount, thread_data_t * connection_fd, response_t * response);
float transfer(int account_from, int account_to, float amount ,thread_data_t * connection_fd, response_t *response);
float operations(operation_t operation, int account_from, int account_to, float amount, thread_data_t * connection_fd, response_t * response);



///// MAIN FUNCTION
int main(int argc, char * argv[])
{
    int server_fd;
    bank_t bank_data;
    locks_t data_locks;

    printf("\n=== SIMPLE BANK SERVER ===\n");

    // Check the correct arguments
    if (argc != 2)
    {
        usage(argv[0]);
    }

    // Configure the handler to catch SIGINT
    setupHandlers();

    // Initialize the data structures
    initBank(&bank_data, &data_locks);

	// Show the IPs assigned to this computer
	printLocalIPs();
    // Start the server
    server_fd = initServer(argv[1], MAX_QUEUE);
	// Listen for connections from the clients
    waitForConnections(server_fd, &bank_data, &data_locks);
    // Close the socket
    close(server_fd);

    // Clean the memory used
    closeBank(&bank_data, &data_locks);

    // Finish the main thread
    pthread_exit(NULL);



    return 0;
}

///// FUNCTION DEFINITIONS

/*
    Explanation to the user of the parameters required to run the program
*/
void usage(char * program)
{
    printf("Usage:\n");
    printf("\t%s {port_number}\n", program);
    exit(EXIT_FAILURE);
}

/*
    Modify the signal handlers for specific events
*/
void setupHandlers()
{
    struct sigaction new_action;
    new_action.sa_handler = onInterrupt;
    sigaction(SIGINT, &new_action, NULL);

}

/* 
    Handler for SIGINT
*/
void onInterrupt(int signal)
{
    interrupt_exit = 1;
}

/*
    Function to initialize all the information necessary
    This will allocate memory for the accounts, and for the mutexes
*/
void initBank(bank_t * bank_data, locks_t * data_locks)
{
    // Set the number of transactions
    bank_data->total_transactions = 0;

    // Allocate the arrays in the structures
    bank_data->account_array = malloc(MAX_ACCOUNTS * sizeof (account_t));
    // Allocate the arrays for the mutexes
    data_locks->account_mutex = malloc(MAX_ACCOUNTS * sizeof (pthread_mutex_t));

    // Initialize the mutexes, using a different method for dynamically created ones
    //data_locks->transactions_mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_init(&data_locks->transactions_mutex, NULL);
    for (int i=0; i<MAX_ACCOUNTS; i++)
    {
        //data_locks->account_mutex[i] = PTHREAD_MUTEX_INITIALIZER;
        pthread_mutex_init(&data_locks->account_mutex[i], NULL);
        // Initialize the account balances too
        bank_data->account_array[i].balance = 0.0;
    }

    // Read the data from the file
    readBankFile(bank_data);
}


/*
    Get the data from the file to initialize the accounts
*/
void readBankFile(bank_t * bank_data)
{
    FILE * file_ptr = NULL;
    char buffer[BUFFER_SIZE];
    int account = 0;
    char * filename = "accounts.txt";

    file_ptr = fopen(filename, "r");
    if (!file_ptr)
    {
        fatalError("ERROR: fopen");
    }

    // Ignore the first line with the headers
    fgets(buffer, BUFFER_SIZE, file_ptr);
    // Read the rest of the account data
    while( fgets(buffer, BUFFER_SIZE, file_ptr) )
    {
        sscanf(buffer, "%d %d %f", &bank_data->account_array[account].id, &bank_data->account_array[account].pin, &bank_data->account_array[account].balance); 
        account++;
    }
    
    fclose(file_ptr);
}
void writeBankFile(bank_t * bank_data)
{
    FILE * file_ptr = NULL;
    int account = 0;
    char * filename = "accounts.txt";


    file_ptr = fopen(filename, "w+");
    if (!file_ptr)
    {
        fatalError("ERROR: fopen");
    }

    // WRITES THE HEADERS
    fprintf(file_ptr,"Account_number PIN BALANCE \n");

    // Read the rest of the account data
    for(int i=0; i<MAX_ACCOUNTS; i++)
    {
        fprintf(file_ptr,"%d %d %f \n", bank_data->account_array[account].id, bank_data->account_array[account].pin, bank_data->account_array[account].balance); 
        account++;
    }
    
    fclose(file_ptr);
}

/*
    Main loop to wait for incomming connections
*/
void waitForConnections(int server_fd, bank_t * bank_data, locks_t * data_locks)
{
    struct sockaddr_in client_address;
    socklen_t client_address_size;
    char client_presentation[INET_ADDRSTRLEN];
    int client_fd;
    pthread_t new_tid;
    thread_data_t * connection_data = NULL;
    int poll_response;
	int timeout = 500;		// Time in milliseconds (0.5 seconds)

    // Create a structure array to hold the file descriptors to poll
    struct pollfd test_fds[1];
    // Fill in the structure
    test_fds[0].fd = server_fd;
    test_fds[0].events = POLLIN;    // Check for incomming data


    while(!interrupt_exit)
    {
            // Get the size of the structure to store client information
            client_address_size = sizeof client_address;

            while (1)
            {
                // ACCEPT
                // Wait for a client connection
                poll_response = poll(test_fds,1,timeout);

                if(poll_response == 0) // nothing to recieve
                {
                    printf(".");
                    fflush(stdout);
                }

                else if (poll_response == -1) // error
                {
                    perror("poll");
                    //exits inner loop
                    break;
                }
                else 
                {
                    printf("A client wants to connect \n");
                    if(test_fds[0].revents & POLLIN)
                    {
                        client_fd = accept(server_fd, (struct sockaddr *)&client_address, &client_address_size);
                        printf("cliente es : %d \n", client_fd);
                        break;
                    }
                }

            }

            if (!interrupt_exit)
            {
                if (client_fd == -1)
                {
                    fatalError("ERROR: accept");
                }
                else {
                    // Get the data from the client

                    inet_ntop(client_address.sin_family, &client_address.sin_addr, client_presentation, sizeof client_presentation);
                    printf("Received incomming connection from %s on port %d\n", client_presentation, client_address.sin_port);

                    // Prepare the structure to send to the thread
                    connection_data = malloc(sizeof(thread_data_t));

                    connection_data->connection_fd = client_fd; // connection
                    printf("In structyure: %d\n", connection_data->connection_fd);

                    connection_data->bank_data = bank_data; // bank data
                    connection_data->data_locks = data_locks; // bank locks



                    // CREATE A THREAD
                    pthread_create(&new_tid, NULL, attentionThread, connection_data);

                }

            }

    }
    // Show the number of total transactions

    printf("The number of Total transactions is : %d ", connection_data->bank_data->total_transactions);
    // Store any changes in the file
    writeBankFile(bank_data);
}

/*
    Hear the request from the client and send an answer
*/
void * attentionThread(void * arg)
{
    // Receive the data for the bank, mutexes and socket file descriptor

    char buffer[BUFFER_SIZE];
    thread_data_t * connection_data = (thread_data_t * ) arg;
    int client_fd = connection_data->connection_fd;
    bank_t * bank_data = connection_data->bank_data;
    locks_t * data_locks = connection_data->data_locks;


    //variables to use 
    int account_from;
    int account_to;
    float amount;
    float balance;

    operation_t operation;
    response_t response;

    printf("In thread: %d\n", client_fd);
    fflush(stdout);


    // Loop to listen for messages from the client
    while(1)
    {
        // Receive the request
        recvString(client_fd,buffer,BUFFER_SIZE);

        // Process the request being careful of data consistency
        sscanf(buffer, "%d %d %d %f", (int*)&operation, &account_from, &account_to, &amount);

        //checks parameters
        if(!checkValidAccount(account_from))
        {
            response = NO_ACCOUNT;
            sprintf(buffer, "%d %f", response ,bank_data->account_array[account_from].balance);
            sendString(client_fd, buffer, strlen(buffer)+1);
            break;
        }
        else if (operation == EXIT){
            response = BYE;
            sprintf(buffer, "%d %f", response ,bank_data->account_array[account_from].balance);
            sendString(client_fd, buffer, strlen(buffer)+1);
            break;
        }
        else {
            balance = operations(operation,account_from,account_to,amount, connection_data, &response);
        }
        
        // Update the number of transactions

        pthread_mutex_lock(&data_locks->transactions_mutex);
        bank_data->total_transactions ++;
        pthread_mutex_unlock(&data_locks->transactions_mutex);
        // Send a reply

        sprintf(buffer, "%d %f", response ,balance);
        sendString(client_fd, buffer, strlen(buffer)+1);
        
    }

    pthread_exit(NULL);
}

float  operations(operation_t operation, int account_from, int account_to, float amount, thread_data_t * connection_fd, response_t * response)
{
    switch(operation)
    {
        case CHECK:
            return check(account_from, connection_fd, response);
        
        case DEPOSIT:
            return deposit(account_from, amount, connection_fd, response);

        case WITHDRAW:
            return withdraw(account_from, amount, connection_fd, response);
        case TRANSFER:
            return transfer(account_from, account_to, amount,connection_fd, response);
        default:
            *response = ERROR;
            return 0;
    }
}

//function to check
float check(int account, thread_data_t * connection_fd, response_t * response )
{
    bank_t  * bank_data = connection_fd->bank_data;
    locks_t * data_locks = connection_fd->data_locks;
    float balance;

    pthread_mutex_lock(&data_locks->account_mutex[account]);
    *response = OK;
    balance = bank_data->account_array[account].balance;
    pthread_mutex_unlock(&data_locks->account_mutex[account]);
    return balance; 
}
//function to deposit
float deposit(int account, float amount, thread_data_t * connection_fd, response_t * response)
{
    bank_t * bank_data = connection_fd->bank_data;
    locks_t * data_locks = connection_fd->data_locks;
    float balance;
    //activates locks
    pthread_mutex_lock(&data_locks->account_mutex[account]);
    balance = bank_data->account_array[account].balance += amount;
    *response = OK;
    pthread_mutex_unlock(&data_locks->account_mutex[account]);

    return balance;
}
//function to withdraw
float  withdraw(int account, float amount, thread_data_t * connection_fd, response_t * response)
{
    bank_t * bank_data = connection_fd->bank_data;
    locks_t * data_locks = connection_fd->data_locks;
    float balance;
    if (bank_data->account_array[account].balance < amount) // checks the balance of accounts
    {
        *response = INSUFFICIENT;
        balance = bank_data->account_array[account].balance;
    }
    else 
    {
        //activate locks
        pthread_mutex_lock(&data_locks->account_mutex[account]);
        balance = bank_data->account_array[account].balance -= amount;
        *response = OK;
        pthread_mutex_unlock(&data_locks->account_mutex[account]);
    }
    return balance;
}
//function to transfer
float transfer(int account_from, int account_to, float amount ,thread_data_t * connection_fd, response_t *response)
{
    bank_t * bank_data = connection_fd->bank_data;
    locks_t * data_locks = connection_fd->data_locks;
    float balance;
    if (bank_data->account_array[account_from].balance < amount) // check the balance of account
    {
        *response = INSUFFICIENT;
        balance =  bank_data->account_array[account_from].balance;
    }
    else
    {
        //activates locks to both accounts
        pthread_mutex_lock(&data_locks->account_mutex[account_from]);


        //check id the other account is blocked, if so, unlock and lock again until the second account is blocked
        while(pthread_mutex_trylock(&data_locks->account_mutex[account_to]))
        {
            pthread_mutex_unlock(&data_locks->account_mutex[account_from]);
            pthread_mutex_lock(&data_locks->account_mutex[account_from]);

        }

        //changes the balance of both accounts
        bank_data->account_array[account_from].balance -= amount;
        
        balance = bank_data->account_array[account_from].balance;
        bank_data->account_array[account_to].balance += amount;

        //unlocks the mutex
        pthread_mutex_unlock(&data_locks->account_mutex[account_from]);
        pthread_mutex_unlock(&data_locks->account_mutex[account_to]);

    }
    return balance;

}
/*
    Free all the memory used for the bank data
*/
void closeBank(bank_t * bank_data, locks_t * data_locks)
{
    printf("DEBUG: Clearing the memory for the thread\n");
    free(bank_data->account_array);
    free(data_locks->account_mutex);
}


/*
    Return true if the account provided is within the valid range,
    return false otherwise
*/
int checkValidAccount(int account)
{
    return (account >= 0 && account < MAX_ACCOUNTS);
}


