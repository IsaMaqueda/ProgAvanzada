
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
// Sockets libraries
#include <netdb.h>
#include <arpa/inet.h>
#include <stdbool.h>
// Custom libraries
#include "sockets.h"
#include "fatal_error.h"

#define BUFFER_SIZE 1024
#define MAX_ENEMIES 4
#define MAX_OPTIONS 4

// Structs to handle character information
typedef struct CharacterInfo {
	char weapon[15];
	int health;
	int dmg;
	int speed;
} char_st;

typedef struct EnemyParty {
	char_st * vilains;
	int number;
} eparty_st;

///// FUNCTION DECLARATIONS
void usage(char * program);

/*Initialize player character with the role that the player chooses*/
void initPlayer(char_st * player);

/*Initialize enemies with randomized stats */
void initEnemies(eparty_st *vil, int enemies);

/*Free memory allocated for the enemies*/
void freeEnemies(eparty_st * vil);

/*Print the instructions of the game*/
void printInstructions();

/*Function that controls how characters react when player chooses to flee*/
int fleeFunction(char_st * player, eparty_st * enemies);

/*Function that controls how characters react when player chooses to attack*/
bool attackFunction(char_st * player, eparty_st * enemies);

/*Function that controls how characters react when player chooses to defend*/
bool defendFunction(char_st *player, eparty_st * enemies);

/*Function that prints some useful indicators about the enemies in the currrent stage*/
void printEnemies(eparty_st * enemies);

/*Give the player an indication of how he is performing in the game*/
void reviewSelf(char_st * player);

/*Function that iterates and controls battle functions until all enemies in stage are defeated*/
bool battleFunction(char_st * player, eparty_st * enemies);

/*Function that randomly generates loot that give buffs or debuffs to the player*/
bool generateChestCont(char_st *player);

/*Apply buff to the player according to the selected role*/
void applyBuffs(char_st *player);

/*Mother function that iterates and receives server messages until the player is defeated*/
void gameOperations(int connection_fd);