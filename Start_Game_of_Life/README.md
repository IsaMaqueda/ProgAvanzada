# CONWAY'S GAME OF LIFE
Program an implementation of Conway's Game of Life, that uses both Threads and OpenMP to parallelize the calculations.

The game is played on its own, with cells arranged in a matrix. The cells can be alive or dead. At every iteration of the game, the cells can remain as they are, die or be born, following these rules:

* Any live cell with fewer than two live neighbours dies, as if caused by underpopulation.
* Any live cell with two or three live neighbours lives on to the next generation.
* Any live cell with more than three live neighbours dies, as if by overpopulation.
* Any dead cell with exactly three live neighbours becomes a live cell, as if by reproduction

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
gcc pgm_image.c -o pgm_image
```

## Running the tests
Move to the folder
To run the tests you first

### Compile using the MakeFile
run the make build automaton

``` 
make
```
### Run the program

To use the program with threds use
use ./pgm_image {iterations} {file} {number of threads }

``` 
./pgm_image 5 ejemplo.pgm 5
```
To use the program with OpenMp
use ./pgm_image {iterations} {file}

``` 
./pgm_image 5 ejemplo.pgm 
```


## Deployment
run the program. Further instructions will follow.

## Data Structures 
* **pixel_struct** stores the value of the pixel
* **image_struct** Structure to store full image data of any size
* **pgm_struct** Structure for an image in PGM format
* **use_threads** Structure to store and use threads

## Description of the program

Program an implementation of Conway's Game of Life, that uses both Threads and OpenMP to parallelize the calculations.

The game is played on its own, with cells arranged in a matrix. The cells can be alive or dead. At every iteration of the game, the cells can remain as they are, die or be born, following these rules: (https://en.wikipedia.org/wiki/Conway%27s_Game_of_Life (Links to an external site.))

* 1. Any live cell with fewer than two live neighbours dies, as if caused by underpopulation.
* 2. Any live cell with two or three live neighbours lives on to the next generation.
* 3. Any live cell with more than three live neighbours dies, as if by overpopulation.
* 4. Any dead cell with exactly three live neighbours becomes a live cell, as if by reproduction.

## Requirements

* Your program must simulate what happens in every iteration and store the current state in an image. You can use other formats if you prefer. In the end you should have as many images as the number of iterations simulated.

You must make two different versions of the program, one that uses OpenMP to parallelize, and another where you create the threads "manually", and where you do the segmentation of the problem on your own. Use the "time" command to measure the speedup that you get from your programs and make sure the parallel version is faster than the sequential one.
* Make sure to wrap around your image, so that a pixel on the left border can be considered a neighbour of a pixel on the right border. Same goes for the top and bottom borders.

* Your program should take two or three arguments from the comand line:

* 1. The number of iterations of the simulation to compute
* 2. The name of the file that contains the initial setup of the cells
* 3. In the case of the program using threads, it should receive a third argument specifying the number of threads to create
## Built With
* [Ubuntu](https://ubuntu.com/) - the operating system
* [Visual Studio Code](https://code.visualstudio.com/) - Text Editor


## Authors
* **Isabel Maqueda** -*Initial Work* 


## License
This project is licensed under the TEC License

## Acknowledgments
* [Guillermo Echeverria](https://github.com/gilecheverria/TC2025) - for the pgm-library code.
