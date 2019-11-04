/*
    Program for a simple chat client
    The server address and port are provided as arguments to the program
    Isabel Maqueda Rolon
    A01652906
    07/10/19
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <netdb.h>
#include "sockets.h"
// Include own sockets library
#include "sockets.h"

#define SERVICE_PORT 8642
#define BUFFER_SIZE 1024

void usage(char * program);
void communicationLoop(int connection_fd);

int main(int argc, char * argv[])
{
    int client_fd;

    printf("\n=== CLIENT PROGRAM ===\n");

    if (argc != 3)
        usage(argv[0]);

    client_fd = connectToServer(argv[1], argv[2]);
    communicationLoop(client_fd);

    // Closing the socket
    printf("Closing the connection socket\n");
    close(client_fd);

    return 0;
}

// Show the user how to run this program
void usage(char * program)
{
    printf("Usage:\n%s {server_address} {port_number}\n", program);
    exit(EXIT_FAILURE);
}

// Do the actual receiving and sending of data
void communicationLoop(int connection_fd)
{
    char buffer[BUFFER_SIZE];
    int chars_read;
    int limit;
    int bid;
    int move;
    char * string;


    printf("Welcome to the blackjack game!\n");
    printf("Enter the top limit for money to bet: ");
    scanf("%d", &limit);

    // Handshake
    // Send a request
    sprintf(buffer, "START:%d", limit);
    send(connection_fd, buffer, strlen(buffer)+1, 0);


    chars_read = receiveMessage(connection_fd, buffer, BUFFER_SIZE);
    if (strncmp(buffer, "READY", 6) != 0)
    {
        printf("Invalid server. Exiting!\n");
        return;
    }
    printf("Game ready!\n");

    int deal2, deal1, deal;
    int client1;
    int client2, client;
    int new;
    
    //sends ok, Handshake
    sprintf(buffer, "OK");
    send(connection_fd, buffer, strlen(buffer)+1, 0);
    
    //Game begins
    printf("The game begins\n the dealer starts dealing the cards. \n The dealers card is: \n ");

    //recieves the cards
    chars_read = receiveMessage(connection_fd, buffer, BUFFER_SIZE);
    
    //the dealer's open card
    string = strtok(buffer, ":");
	if(strncmp(string,"DEALER1",8) != 0 )
	{
            printf("Invalid DEALER. Exiting!\n");
            return;
	}
    
    //separates string
    string = strtok(NULL, ":");
    deal1 = atoi(string);
    printf("Dealer's open card: %d\n", deal1);
    
    //recieves the dealers only closed card

    string = strtok(NULL, ":");
	if(strncmp(string,"DEALER2",8) != 0 )
	{
            printf("Invalid DEALER2. Exiting!\n");
            return;
	}
    
    string = strtok(NULL, ":");
    deal2 = atoi(string);

    printf("Your cards are: ");
    //recieves the client's open card

    string = strtok(NULL, ":");
	if(strncmp(string,"CLIENT1",8) != 0 )
	{
            printf("Invalid CLIENT1. Exiting!\n");
            return;
	}
    
    string = strtok(NULL, ":");
    client1 = atoi(string);
    printf("Client's open card: %d \n", client1);

    //recieves the Cient's closed card

    string = strtok(NULL, ":");
	if(strncmp(string,"CLIENT2",8) != 0 )
	{
            printf("Invalid CLIENT2. Exiting!\n");
            return;
	}
    
    string = strtok(NULL, ":");
    client2 = atoi(string);
    printf("Client's closed card: %d \n", client2);

    client = client1 + client2;
    deal = deal1 + deal2;


    //starts the betting
    while(1)
    {
        // Send a request
        printf("Enter a bid: ");
        scanf("%d", &bid);
        sprintf(buffer, "BID:%d:", bid);

	    printf("Enter what you want to do: 1. HIT, 2. STAND: \n");
	    scanf("%d",&move);


	    if (move == 1)
	    { 
	        //ads hit to the string in buffer
	        strcat(buffer,"HIT:");
	        //sends the bet and hit
	        send(connection_fd, buffer, strlen(buffer)+1, 0);
	        
	        //recieves the new card
	        chars_read = receiveMessage(connection_fd, buffer, BUFFER_SIZE);
        	string = strtok(buffer, ":");
		    if(strncmp(buffer,"NEW",8) != 0 )
		    {
        		printf("Invalid NEW. Exiting!\n");
        		return;
		    }
        
	        string = strtok(NULL, ":");
	        new = atoi(string);
            client = client + new;
	        printf("New card : %d \n", new);
	        if(client > 21) // if client cards more than 21, the program closes
		    {
			    break;
		    }
	    }
	    else 
	    {
	        strcat(buffer,"HOLD:");
	        send(connection_fd, buffer, strlen(buffer)+1, 0);
	        chars_read = receiveMessage(connection_fd, buffer, BUFFER_SIZE);
		    break;
	    }
	    
	
    }
	printf("The dealer's cards are %d, %d \n His total count is: %d \n", deal1, deal2, deal);
	printf("Your total count was: %d \n", client);

    //compares who won 
    if((client >= deal) && (client <= 21))
	{
	    printf("YOU WON!!\n");
	}
	else
	{
		printf("YOU LOST!!\n");
	}
	
	sprintf(buffer, "OVER");
	send(connection_fd, buffer, strlen(buffer)+1, 0);

    chars_read = receiveMessage(connection_fd, buffer, BUFFER_SIZE);
    printf("Money %s \n", buffer);

    // Close the connection
    send(connection_fd, "BYE", 4, 0);
    chars_read = receiveMessage(connection_fd, buffer, BUFFER_SIZE);
    printf("The server sent me: '%s'\n", buffer);
}
