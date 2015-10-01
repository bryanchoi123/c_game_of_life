README:

Parallelization Strategy:
	My parallelization strategy is to try to divide the work evenly among all the threads. Any extra work that is left over is given to the root thread to handle. Each thread will calculate their part of the next iteration and store it in a temporary array. Once each thread is done, it will call MPI_Barrier() to ensure that all the threads will synchronize before starting the next iteration.

To execute program:
	The command is as follows:
		mpirun –n p ./mp6 file1 file2 m n
	Where:
		- p is the number of processes to run the program with 
		- file1 a text file containing the number of rows and columns on 
	their first line and the initial state of the cells on the subsequent lines
		- file2 the text file to print the output of the program
		- m the total number of iterations to run Life
		- n how many iterations to run Life until the board’s current state 
	is printed (i.e. printed every nth iteration)
