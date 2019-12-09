/*
    Client program to access the accounts in the bank
    This program connects to the server using sockets

    Isabel Maqueda Rolon A01652906
	Eduardo Badillo A0120716
    09/12/2019
*/
#include "client.h"

///// MAIN FUNCTION
int main(int argc, char * argv[])
{
    int connection_fd;

    printf("\n=== GAME CLIENT PROGRAM ===\n");

    // Check the correct arguments
    if (argc != 3)
    {
        usage(argv[0]);
    }

    // Start the server
    connection_fd = connectSocket(argv[1], argv[2]);
	// Use the bank operations available
    gameOperations(connection_fd);
    // Close the socket
    close(connection_fd);

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

//Initializes the character you want to play with stats
void initPlayer(char_st * player) {
	int choice;
	printf("Choose Role to play\n");
	sleep(1);
	printf("Mages have high damage output but lesser health\n");
	sleep(3);
	printf("Warriors have less damage output but greater health\n");
	sleep(3);
	printf("Healers heal over time and have a decent damage output\n");
	sleep(3);
	printf("1: Mage, 2: Warrior, 3: Healer\n");
	scanf("%d",&choice);
	switch (choice) {

		case 1:
			printf("Mage chosen\n");
			player->dmg = 20;
			player->speed = 8;
			player->health = 40;
            strcpy(player->weapon, "Staff");
			break;
		case 2:
			printf("Warrior chosen\n");
			player->dmg = 16;
			player->speed = 18;
			player->health = 70;
			strcpy(player->weapon, "Sword");
			break;

		case 3:
			printf("Healer chosen\n");
			player->dmg = 14;
			player->speed = 14;
			player->health = 60;
			strcpy(player->weapon, "Wand");
			break;

		default:
			printf("Choice not registered\n");
			initPlayer(player);
            break;
	}
    fflush(stdin);
}

void initEnemies(eparty_st *vil, int enemies) {
	//allocate the memory necesary
	vil->vilains = malloc(enemies * sizeof(char_st));

    //initializes Random seed for the random
	srand(time(NULL));

	vil->number = 0;

	for (int i = 0; i < enemies; i++) {
		vil->vilains[i].dmg = rand() % (20 + 1 - 5) + 5;
		vil->vilains[i].health = rand() % (25 + 1 - 15) + 15;
		vil->vilains[i].speed = rand() % (19 + 1 - 2) + 2;
		vil->number++;
	}
}
void freeEnemies(eparty_st * vil) 
{
	free(vil->vilains);
}

void printInstructions() {
	printf("                       ---  TOWER OF DOOM  ---            \n");
	sleep(2);
	printf("In this game you climb the tower and defeat hazards as they appear\n");
	sleep(3);
	printf("Your score will be the number of floors traversed.\n");
	sleep(3);
	printf("In each level you can encounter enemies or a chest with random stuff\n");
	sleep(3);
	printf("Enemies can be fought and the stage cleared when they are all defeated\n");
	sleep(3);
	printf("When battling you can attack, defend or flee.\n");
	sleep(3);
	printf("When attacking enemies, your weapon gets less effective\n");
	sleep(2);
	printf("But when you defend, the enemies' weapons worsen if you block effectively\n");
	sleep(3);
	printf("In combat you can use simple commands: ATTACK, DEFEND, FLEE \n");
	sleep(2);
}

int fleeFunction(char_st * player, eparty_st * enemies) {
	// Only enemies with higher speed attack, damage is halved
	// Player loses speed if he cant flee

	printf("They try to stop you from escaping!\n");
	sleep(3);
	for (int i = 0; i < enemies->number; i++) {
		
		if (enemies->vilains[i].speed > player->speed) {
			player->health = player->health - (enemies->vilains[i].dmg / 2);
			player->speed -= 2;
			printf("You couldn't flee, you received %d damage\n", (enemies->vilains[i].dmg / 2));

			if(player->health < 1) {
				printf("Enemy %d delivered the killing blow !\n", i + 1);
				return -1;
			}
			return 0;
		} 
	}
	printf("But you fled succesfully\n");
	sleep(2);
	return 1;
}

// Function if player chooses to attack
bool attackFunction(char_st * player, eparty_st * enemies) {

	printf("You enter an offensive stance!\n");
	sleep(2);
	int enemiesLeft = enemies->number;
	
	// Characters attack first according to their speed stat
	for (int i = 0; i < enemies->number; i++) {

		if (enemies->vilains[i].health > 0) {

			printf("Enemy %d and you engage in battle!\n", i + 1);
			sleep(2);

			if (player->speed >= enemies->vilains[i].speed) {
				enemies->vilains[i].health -= player->dmg;

				printf("You stroke first ! Dealing %d damage\n", player->dmg);
				sleep(3);
				// If enemy is defeated before retaliating no damage is given to player 
				if (enemies->vilains[i].health > 0) {
					printf("Enemy %d responded, dealing %d damage\n", i + 1, enemies->vilains[i].dmg);
					player->health -= enemies->vilains[i].dmg;
				}
				else {
					printf("Enemy %d is defeated before retaliating!\n", i + 1);
				}

				
			}
			else {

				printf("Enemy %d stroke first dealing %d damage ! \n",i + 1,  enemies->vilains[i].dmg);
				sleep(3);
				player->health -= enemies->vilains[i].dmg;
				if (player->health > 0) {
					printf("You retaliated against your attacker, dealing %d damage !\n ", player->dmg);
					enemies->vilains[i].health -= player->dmg;
					sleep(2);
				}
				else {
					printf("The strike was fatal! \n");
					sleep(1);
				}

				
			}

			if (enemies->vilains[i].health <= 0) {
				sleep(1);
				printf("Enemy %d was deafeated!\n", i + 1);
				sleep(1);
				enemiesLeft = enemiesLeft - 1;
				printf("Enemies left %d\n", enemiesLeft);
			}
			player->dmg = player->dmg - 1;
		}
		else {
			enemiesLeft -= 1;
		}
		if (player->health <= 0) {
			return false;
		}
	}
	if (enemiesLeft == 0)
		enemies->number = 0;
	return true;
}

// Function if player chooses defense
bool defendFunction(char_st *player, eparty_st * enemies) {

	printf("You enter a defensive stance !\n");
	sleep(2);

	for (int i = 0; i < enemies->number; i++) {

		if (enemies->vilains[i].health > 0) {

			printf("Enemy %d attacks...\n", i + 1);
			sleep(1);
			// Player receives damage if lower attack stat
			if (enemies->vilains[i].dmg > player->dmg) {
				printf("You couldn't defend! Damage taken : %d\n", enemies->vilains[i].dmg);
				player->health -= enemies->vilains[i].dmg;
				player->dmg -= 1;

				sleep(2);
			}
			else {
				printf("But you defended yourself succesfully!\n");
				enemies->vilains[i].speed -= 2;
				sleep(2);
			}

			enemies->vilains[i].dmg -= 2;

			// Player is defeated 
			if (player->health <= 0) {
				printf("That was the killing blow!\n");
				return false;
			}
		}

	}
	return true;
}


void printEnemies(eparty_st * enemies) {
	for (int i = 0; i < enemies->number; i++) {
		//printf("Enemy %d AP = %d\n", i + 1, enemies->vilains[i].dmg);
		//printf("Enemy %d SP = %d\n", i + 1, enemies->vilains[i].speed);

		printf("Enemy %d HP  =", i + 1);
		for (int j = 0; j < enemies->vilains[i].health; j++) {
			printf("|");
		}
		printf("\n");
	}
}


// Function to give a description of player health
void reviewSelf(char_st * player) {
	sleep(2);
	printf("You sit down and take a moment to check your injuries and equipment\n");

	int h = player->health;

	printf("Health %d\n", player->health);
	sleep(1);
	printf("Damage %d\n", player->dmg);
	sleep(1);
	printf("Speed %d\n", player->speed);
	sleep(1);
	//int d = player->dmg;

	if (h > 20 && h < 30) {
		printf("You see you have clearly sustained some important damage and should proceed more carefully\n");
	}
	else if (h>10 && h < 20) {
		printf("You see your injuries are more important than you thought...\n");
	}
	else if (h < 10) {
		printf("Your body is badly hurt, you can ascertain this journey will end quickly,\n");
	}
	else {
		printf("You are well and ready to continue your joruney\n");
	}
	sleep(2);

}


// Returns true if the player defeats all enemies in the stage or flees 
// Returns false if it dies
bool battleFunction(char_st * player, eparty_st * enemies) {
	
	char decision[10];
	char getsEater[10];
	fgets(getsEater, 10, stdin);
	bool alive = true;
	int fled;

	printf("You see %d enemies stand in your way\n", enemies->number);
	
	while (alive) {
		printEnemies(enemies);
		printf("ACTION: ");
		
            fgets(decision, 10, stdin);
	        decision[strlen(decision) - 1] = '\0';
			
			// FLEE
			if ((strcmp(decision,"FLEE") == 0) || (strcmp(decision,"flee")==0)) {
				fled = fleeFunction(player, enemies);
				if (fled < 0) { // Player died while escaping
					alive = 0;
				}
				else if (fled==0){ // Player could not escape 
					alive = 1;
				}
				else { // Player escaped
					break;
				}
			}
			// ATTACK
			else if ((strcmp(decision,"ATTACK")==0) || (strcmp(decision,"attack")==0)) {
				alive = attackFunction(player, enemies);
			} // DEFEND
			else if ((strcmp(decision,"DEFEND")==0) || (strcmp(decision,"defend")==0)) {
				alive = defendFunction(player, enemies);
			} // INCORRECT OPTION
			else {
				printf("\n");
				printEnemies(enemies);
				printf("\n");
				reviewSelf(player);
				printf("\n");
				printf("The command you entered was incorrect.\n");
			}

			if (enemies->number == 0 && alive) {
				sleep(2);
				printf("No more enemies in sight\n");
				sleep(1);

				printf("Congratulations! Stage cleared!\n ");
				sleep(1);
				break;
			}
	}

	return alive;
}

bool generateChestCont(char_st *player) {
	char c;
	printf("Do you open it? y/n \n");
	scanf(" %c", &c);

	if (c == 'y' || c == 'Y') {
		srand(time(NULL));

		int opt = rand() % 4;
		switch (opt)
		{
		case 0:
			player->health += 20;
			printf("The chest contained a potion of health, your health is now at : %d\n", player->health);
			break;
		case 1:
			player->health -= 20;
			printf("The chest exploded upon contact! Your health is now at : %d\n", player->health);
			if (player->health < 1)
				printf("The blast was fatal!\n");
			break;
		case 2:
			player->dmg += 10;
			printf("The chest contained an upgrade for your weapon, your damage is: %d\n", player->dmg);
			break;
		case 3:
			player->speed += 4;
			printf("The chest contained an inspirational short tale, you feel inspired and gain some speed points\n");
			break;
		case 4:
			printf("The chest was empty\n");
			break;
		default:
			break;
		}
	}

	if (player->health > 0) {
		return true;
	}
	return false;
}

void applyBuffs(char_st *player) {

	// If player is a healer replenishes some health
	if (strcmp(player->weapon, "Wand") == 0) {
		player->health = player->health + 20;
	}  // If player is a warrior replenishes some speed
	else if (strcmp(player->weapon, "Sword") == 0) {
		player->speed = player->speed + 2;
	}
	else { // If player is a mage replenishes some damage points
		player->dmg = player->dmg + 2;
	}
}

void gameOperations(int connection_fd)
{


    char buffer[BUFFER_SIZE];
	char * storyline = malloc(200 * sizeof(char));
    storyline = malloc(200 * sizeof(char));
	bool outcome = true;

	eparty_st vilains;
	int enemies;
	int type;
	int stage = 0;


	// Initialize player information
	char_st playerChar;
	
	char c;
	printf("\n=== GAME STARTED ===\n");
	printf("Read the instructions? y/n \n");
	c = getchar();

	if(c=='y' || c=='Y')
		printInstructions();
	initPlayer(&playerChar);

	printf("You enter the tower with your trustworthy %s at hand\n", playerChar.weapon);
	sleep(2);
    while (outcome)
    {
		
		// RECV
		// Receive the string of level
		if (!recvString(connection_fd, buffer, BUFFER_SIZE))
		{
			printf("Server closed the connection\n");
			break;
		}

		// Extract the data
		sscanf(buffer, "%d %d  %[^\n]s", &type, &enemies, storyline);

		// Print what is happening
		stage++;
		printf("STAGE No: %d ___________________________________________________________________________________________________\n", stage);
		printf("%s\n", storyline);
		sleep(4);

		if (type == 0) { // If enemies are in stage
			initEnemies(&vilains, enemies);
			outcome = battleFunction(&playerChar, &vilains);
		}
		else {  // Random chest scenario
			outcome = generateChestCont(&playerChar);
		}

		if (outcome) {
			sleep(2);
			printf("You cleared the stage! Advancing to the next!\n");
			sleep(2);
			applyBuffs(&playerChar);
			reviewSelf(&playerChar);
			printf("You climb on to the next stage...\n");
			sleep(2);

			sprintf(buffer, "ALIVE");
			sendString(connection_fd, buffer, strlen(buffer) + 1);
			
		}
		else {
			printf("You were defeated!\n");
			sprintf(buffer, "DEAD");
			sendString(connection_fd, buffer, strlen(buffer) + 1);
		}
		// Free memory 
		freeEnemies(&vilains);

    }

	recvString(connection_fd, buffer, BUFFER_SIZE);

}
