#include <stdio.h>
#include <stdlib.h>

#include <mpi.h>


int main(int argc, char **argv)
{
	int ierr, processID;

	// start MPI
	ierr = MPI_Init(&argc, &argv);

	ierr = MPI_Comm_rank(MPI_COMM_WORLD, &processID);
	printf("Hi, I'm process %d\n", processID);

	ierr = MPI_Finalize();

	printf("All done. I'm %d\n", processID);

	return 0;
}
