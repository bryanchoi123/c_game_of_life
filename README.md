# Conway's Game of Life
Conway's Game of Life for Parallel Programming Course

## Description
The purpose of this program was to make use of the MPI library in C.

To execute this program, compile and run with the following parameters:

```
./program input output m n
```

Where:
- **input** is a file containing the number of rows and columns on their first line and the initial state of the cells on the subsequent lines
- **output** is where the program should print the output i.e. the board configurations
- **m** is the total number of iterations to run
- **n** is how many iterations to run until the board’s current state	is printed (printed every nth iteration)
