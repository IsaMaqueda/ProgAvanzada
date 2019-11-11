# Simple Bank 
This program simulates a network consisting on the central server of a bank, and client programs that would be running on the ATM machines, connecting to the central server to validate operations.

Complete the server program to create a bank system that can receive connections via sockets, and that creates threads to serve each client that connects.

## Getting Started
These instructions will get you a copy of the project up and running on your local machine for development and testing purposes. See deployment for notes on how to deploy the project on a live system.

### Prerequisites
You will need to install the GNU compiler and a Make build automaton
``` 
sudo apt instal ggc
```
``` 
sudo apt instal make
```
### Installing
A step by step series of examplesto run the project

Install ggc

``` 
sudo apt instal ggc
```
Install make
``` 
sudo apt instal make
```
and then to run 
``` 
gcc client.c -o client
```

## Running the tests
Move to the folder
To run the tests you first

### Compile using the MakeFile
run the make build automaton

``` 
make
```
### Run the Server
use ./server {port_number}

``` 
./server 8989
```
### Run the Client
Open another tab in terminal and run
./client {server_address} {port}

``` 
./client localhost 8989
```

## Deployment
run the program. Further instructions will follow.

## Data Structures 
* **bank_t** stores the data of the bank. It has an int to count the total number of transactions performed, and an array of floats that store the balance of the accounts. Each account is identified by the index in that array.
* **locks_t** has the mutex locks for the variables in the previous structure. One for the total number of transactions, and one for each of the accounts.
* **thread_data_t** has pointers to the previous two, plus the file descriptor of the socket to communicate with the client. This is the one that is passed to each thread as a parameter.

## Description of the program

The client will pass messages to the server consisting of three ints and a float. The first int is a code indicating the type of operation to make (available operations are listed in bank_codes.h as an enum operation_t), the second is the source account number (when taking money out), the third parameter is the destination account (when depositing money), and the fourth is the amount to use for the operation. If the operation selected does not need an amount (for example check balance), and mount of 0 is sent. If an operation does not need an account (for example withdraw does not need a destination account), then an account number of 0 is sent.

The server will reply with a code also listed in bank_codes.h as response_t, and the new balance of the account. If the response does not use a balance, send 0.

## Requirements
* Your program must use sockets, polling, signal handlers and threads.
* When a client connects, a new thread is created. It should receive a structure with the bank data.
* Make sure to use the corresponding locks to avoid data corruption, mainly during a transfer.
* Use functions to keep your code well organized.
* When the server is closed with Ctrl-C, print the total number of transactions performed in the session and write the account balances in the file "accounts.txt" to save any operations done. Make sure to use the exact same format for the file, so that it can be used again by the program.

## Built With
* [Ubuntu](https://ubuntu.com/) - the operating system
* [Visual Studio Code](https://code.visualstudio.com/) - Text Editor


## Authors
* **Isabel Maqueda** -*Initial Work* 

## License
This project is licensed under the TEC License

## Acknowledgments
* [Guillermo Echeverria](https://github.com/gilecheverria/TC2025) - for the sockets code.
