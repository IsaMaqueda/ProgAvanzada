//Isabel Maqueda A01652906
// 22/08/2019
//Advanced Programming

#include <stdio.h>
#include <stdlib.h>

#define SIZE 30

 //fill an array with random numbers between 1-100
void RandArray(int arr[SIZE])
{
    for (int i=0;i<SIZE;i++)
    {
        int x = (rand() %(100 - 1 + 1) ) +1; // random number
        arr[i]= x;  //saves in array
    }
}

//compute the difference
int Difference(int x, int y)
{
    return x-y;
}
void PrintinFile(int arr[SIZE], int y)
{
    FILE * file_ptr; //creates a pointer to the file
    file_ptr = fopen("differences.txt","w"); // opens the file

    if(file_ptr){ //checks if it open the file
        for (int i=0;i<SIZE;i++) //moves int he array
        {
            int w = Difference(arr[i],y); //difference
            fprintf(file_ptr,"%d\t", w);


        }
    }
    else { //marks error in opening file
            printf("Failed to open file");

    }
    fclose(file_ptr);//closes the file


}

int main(){

    int arr[SIZE]; // creates the array
    int y; //the number for the difference
    RandArray(arr); //fills the array with random numbers

    printf("Provide number between 1 and 100: "); // asks for a number
    scanf("%d", y); // saves the number in the variable
    PrintinFile(arr, y); // print the difference in a txt file
    return 0;
}
