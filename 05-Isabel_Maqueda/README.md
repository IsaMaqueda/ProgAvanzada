# Blackjack Game 
This program uses sockets to simulate a client and a server to play a blackjack game

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

## Built With
* [Ubuntu](https://ubuntu.com/) - the operating system
* [Visual Studio Code](https://code.visualstudio.com/) - Text Editor


## Authors
* **Isabel Maqueda** -*Initial Work* 

## License
This project is licensed under the TEC License

## Acknowledgments
* [Guillermo Echeverria](https://github.com/gilecheverria/TC2025) - for the sockets code.
