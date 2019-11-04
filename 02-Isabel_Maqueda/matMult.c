/*
Isabel Maqueda Rolon A01652906
H2: Array multiplication
Advanced Programming

02/09/2019

*/
#include "stdio.h"
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

typedef struct {
    float i; // no. of rows
    float j; // no. of columns
    double** mtrx; // ptr to matrix
} matrix;

//Allocates memory for the r*c matrix of the struct
double** memAlloc(double** matrix, int r, int c)
{
    matrix=malloc(sizeof(int)*r*c);
    if(matrix!=NULL)
    {
    	for(int i=0; i<r;i++)
    	{
    		matrix[i] = malloc(sizeof(int)*c);
            if(matrix[i]==NULL)
            {
            	return NULL;
            }
        }
    }
    return matrix;
}

// Reads matrix from file and adds it to a matrix struct
matrix* fillMatrix(char fName[50]){
	FILE *file_ptr;
	file_ptr = fopen(fName, "r");
	
	matrix * mat = malloc(sizeof(matrix));
	mat->mtrx = NULL;
	int r, c; 
	fscanf(file_ptr, "%1d", &r);
	fscanf(file_ptr, "%1d", &c);
	mat->mtrx = memAlloc(mat->mtrx, r, c);
	double x;

	for(int i=0; i<r; i++){
		for(int j=0; j<c; j++){
			fscanf(file_ptr,"%lf",&x); 
     			mat->mtrx[i][j] = x;
     			printf("data[%d][%d]=%lf\n",i,j,mat->mtrx[i][j]);  
		}
	}
	mat->i = r;
	mat->j = c;
	fclose(file_ptr);
	return mat;
}

//Multiplies matrix from struct a with matrix from struct b, creates a new one
matrix* matMult(matrix* A, matrix* B){
	matrix * mat = malloc(sizeof(matrix));
	double** mtrx = NULL;
	mtrx = memAlloc(mtrx, A->i, B->j);
	double sum;
	
	for (int i = 0; i < A->i; i++) { // Row of matrix A
      		for (int j = 0; j < B->j; j++) {  //Columns of matrix B
        		for (int k = 0; k < B->i; k++) { // Rows of matrix B
         			sum = sum + (A->mtrx[i][k]*B->mtrx[k][j]);
        		}
        		mtrx[i][j] = sum;
          		sum = 0;
      	 	}
    	}
	mat->mtrx = mtrx;
	mat->i = A->i;
	mat->j = B->j;
	return mat;
}

// Print a given matrix struct to a given file
void printMatrix(matrix* M, char fName[50]){
	FILE *f_ptr;
	f_ptr = fopen(fName, "w");
	for(int i=0; i<M->i; i++){
		for(int j=0; j<M->j; j++){
			if(f_ptr){
				double x = M->mtrx[i][j];
				printf("%f ",x);  
				fprintf(f_ptr, "%f ",x);  
			}
		}
		fprintf(f_ptr, "\n");
		printf("\n");  
	}
	fclose(f_ptr);
}

// checks wether 2 matrices can be multiplied with each other
bool canMult(matrix* A, matrix* B){
	if(A->j==B->i){
		return true;
	}
	return false;
}

int main(int argc, char * argv[]){

	char matA[50];
	char matB[50];
	char res[50];
	int opt;
	
	while((opt=getopt(argc, argv, "1:2:r:"))!=-1){
		switch(opt)
		{
		case '1':
		 strncpy(matA, optarg, 50);
		 break;
		case '2':
		 strncpy(matB, optarg, 50);
		 break;
		case 'r':
		 strncpy(res, optarg, 50);
		 break;
		case '?':
		 printf("Unknown option %c\n", opt);
		 break;
		}		 
	}

	matrix* matrixA = fillMatrix(matA);
	matrix* matrixB = fillMatrix(matB);
	matrix* matrixC = malloc(sizeof(matrix));

	if(canMult(matrixA, matrixB)){
		matrixC = matMult(matrixA, matrixB);
		printMatrix(matrixC, res);
	} else {
		printf("Cannot multiply matrices");

	}

	free(matrixA);
	free(matrixB);
	free(matrixC);
	return 0;
}
