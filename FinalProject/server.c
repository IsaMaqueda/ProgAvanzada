/*
Program for the server of Final project, that is a videogame based on ZORK
Isabel Maqueda Rolon A01652906
Eduardo Badillo Alvarez A01020716

9/12/2019
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
// Signals library
#include <errno.h>
#include <signal.h>
// Sockets libraries
#include <netdb.h>
#include <sys/poll.h>
// Posix threads library
#include <pthread.h>

//custom libraries
#include "sockets.h"
#include "fatal_error.h"

#define MAX_STAGES 2
#define BUFFER_SIZE 1024
#define MAX_QUEUE 5


////// structure definitions


//data for a single stage
typedef struct stage_struct
{
    int id;
    int type;
    int enemies;
    char  * desc;
} stage_t;

//data for the game 
typedef struct game_struct
{
    //store the total of levels played
    int total_levels;

    //an array of the stages
    stage_t * stage_array;
} game_t;

//structure of the mutexes to keep the data consistent
typedef struct locks_struct {

    //mutex for the number of levels played
    pthread_mutex_t levels_mutex;
    //mutex array for the number of stages 
    pthread_mutex_t * stage_mutex;

} locks_t;

//data that will be sent to each structure
typedef struct data_struct {
    //the file descriptor of the socket
    int connection_fd;
    //a pointer to the stages
    game_t * game_data;
    //a pointer to the locks
    locks_t * data_locks;
} thread_data_t;

//global variables for signal handling
int interrupt_exit = 0;

//FUNCTION DECLARATIONS
void usage(char * program);
void setupHandlers();
void onInterrupt(int signal);
sigset_t setupMask();
void unsetMask(sigset_t old_set);
void initGame(game_t * game_data, locks_t * data_locks);
void readStageFile(game_t * game_data);
void waitForConnections(int server_fd, game_t * game_data, locks_t * data_locks);
void closeGame(game_t * game_data, locks_t * data_locks);
void * attentionThread(void * arg);



///// MAIN FUNCTION
int main(int argc, char * argv[])
{
    int server_fd;
    game_t game_data;
    locks_t data_locks;

    printf("\n=== SIMPLE GAME SERVER ===\n");

    // Check the correct arguments
    if (argc != 2)
    {
        usage(argv[0]);
    }

    // Configure the handler to catch SIGINT
    setupHandlers();

    //Configure Mask to block all other signals exept SIGINT
    sigset_t old;

    old = setupMask();

    // Initialize the data structures
    initGame(&game_data, &data_locks);

	// Show the IPs assigned to this computer
	printLocalIPs();
    // Start the server
    server_fd = initServer(argv[1], MAX_QUEUE);
	// Listen for connections from the clients
    waitForConnections(server_fd, &game_data, &data_locks);
    // Close the socket
    close(server_fd);

    // Clean the memory used
    closeGame(&game_data, &data_locks);

    //allow the computer to catch other signals
    unsetMask(old);

    // Finish the main thread
    pthread_exit(NULL);

    return 0;
}

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
    //the signal catches SIGINT
    struct sigaction new_action;
    new_action.sa_handler = onInterrupt;
    sigaction(SIGINT, &new_action, NULL);

}
// Modify the signal mask
sigset_t setupMask()
{
    sigset_t new_set;
    sigset_t old_set;

    //Block all signals
    sigfillset(&new_set);

    //Remove the signal SIGNIT from the block
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
    printf("Unsetting Maks\n");
    sigprocmask(SIG_SETMASK, &old_set, NULL);
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
    This will allocate memory for the stages and for the mutexes
*/
void initGame(game_t * game_data, locks_t * data_locks)
{
    // Set the number of levels played at the start
    game_data->total_levels = 0;

    // Allocate the arrays in the structures
    game_data->stage_array = malloc(MAX_STAGES * sizeof (stage_t));
    //initializes the space for the desc
    for(int i = 0; i<MAX_STAGES;i++){
        game_data->stage_array[i].desc = malloc(200 * sizeof(char));
    }


    // Allocate the arrays for the mutexes
    data_locks->stage_mutex = malloc(MAX_STAGES * sizeof (pthread_mutex_t));

    // Initialize the mutexes, using a different method for dynamically created ones

    pthread_mutex_init(&data_locks->levels_mutex, NULL);

    for (int i=0; i<MAX_STAGES; i++)
    {
        pthread_mutex_init(&data_locks->stage_mutex[i], NULL);
    }

    printf("Memory is allocated\n");
    // Read the data from the file
    readStageFile(game_data);
}
void readStageFile(game_t * game_data)
{
    FILE * file_ptr = NULL;
    char buffer[BUFFER_SIZE];
    int i = 0;
    char * filename = "stage.txt";

    //open file
    file_ptr = fopen(filename, "r");
    //check if file opens 
    if (!file_ptr)
    {
        fatalError("ERROR: fopen");
    }


    // Ignore the first line with the headers
    fgets(buffer, BUFFER_SIZE, file_ptr);
    // Read the rest of the account data
    while(fgets(buffer, BUFFER_SIZE, file_ptr) )
    {
        sscanf(buffer, "%d %d %d %[^\n]s", &game_data->stage_array[i].id, &game_data->stage_array[i].type, &game_data->stage_array[i].enemies,game_data->stage_array[i].desc);
        printf("Printing the stages: %d\n", i);
        printf("%d %d %d %s \n", game_data->stage_array[i].id, game_data->stage_array[i].type, game_data->stage_array[i].enemies,game_data->stage_array[i].desc); 
        i++;
    }
    
    fclose(file_ptr);
}

void waitForConnections(int server_fd, game_t * game_data, locks_t * data_locks)
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

    //while it is not interrupted
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
                    printf("In structure: %d\n", connection_data->connection_fd);

                    connection_data->game_data = game_data; // game data
                    connection_data->data_locks = data_locks; // game locks



                    // CREATE A THREAD
                    pthread_create(&new_tid, NULL, attentionThread, connection_data);

                }

            }

    }
    // Show the number of total levels played while server is active

    printf("The number of Total Levels played is : %d \n", connection_data->game_data->total_levels);

}

//clears memory 
void closeGame(game_t * game_data, locks_t * data_locks)
{
    printf("DEBUG: Clearing the memory for the thread\n");
    for(int i = 0; i<MAX_STAGES;i++)
    {
        free(game_data->stage_array[i].desc);
    }
    printf("The desc is freed\n");
    free(game_data->stage_array);
    printf("game_data is freed\n");
    free(data_locks->stage_mutex);
    printf("locks is freed\n");
}

/*
    Hear the request from the client and send an answer
*/
void * attentionThread(void * arg)
{
    // Receive the data for the game, mutexes and socket file descriptor

    char buffer[BUFFER_SIZE];
    thread_data_t * connection_data = (thread_data_t * ) arg;
    int client_fd = connection_data->connection_fd;
    game_t * game_data = connection_data->game_data;
    locks_t * data_locks = connection_data->data_locks;

    // Initialize the random seed
    srand(time(NULL));

    //variables to use 
    int id_stage;
    int id;
    int type;
    int enemies;
    char * desc1 = malloc(200 * sizeof(char));
    printf("In thread: %d\n", client_fd);
    fflush(stdout);

    // Loop to listen for messages from the client
    while(1)
    {

        //selects a random stage to send to client
        id_stage = rand() % MAX_STAGES;
        
        //gets the stage id, type, enemies and description

        printf("THe random number is: %d\n",id_stage);

        id = game_data->stage_array[id_stage].id;
        type = game_data->stage_array[id_stage].type;
        enemies = game_data->stage_array[id_stage].enemies;

        strcpy(desc1,game_data->stage_array[id_stage].desc);

        printf("id= %d, type: %d, enemies: %d \nDesc: %s", id, type, enemies , desc1);

         //Send the stage to use
        sprintf(buffer, "%d %d %s", type, enemies, desc1);
        sendString(client_fd, buffer, strlen(buffer)+1);

        
        // Update the number of levels played
        pthread_mutex_lock(&data_locks->levels_mutex);
        game_data->total_levels ++;
        pthread_mutex_unlock(&data_locks->levels_mutex);
        

        // Receive the response of client 
        recvString(client_fd,buffer,BUFFER_SIZE);

        //checks condition of player, if player is alive, continue playing, else close programS
        if(strncmp(buffer,"ALIVE",6) == 0)
        {
            printf("The Player %d is alive", client_fd);

        }
        else if(strncmp(buffer,"DEAD",5) == 0)
        {
            printf("The Player  %d is dead", client_fd);
            break;
        }else
        {
            break;
        }
    }

    sprintf(buffer,"BYE");
    sendString(client_fd, buffer, strlen(buffer)+1);

    pthread_exit(NULL);
}