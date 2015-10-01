#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include <time.h>
#include <sys/time.h>

/**   FUNCTIONS  **/

// MPI version
void iterate(int argc, char **argv);
int step_MPI(int row, int col);

// serial version
void step();

int getLiveNeighbors(int r, int c);

/**   HELPERS   **/
void initializeParameters(int argc, char **argv);
void readMaze(char *fileName);
void writeGeneration(int **currentBoard);
void printMaze();
void freeData();

#define send_tag 100
#define recieve_tag 101

#define DEAD 0
#define ALIVE 1

int totalIterations, printIterations;
int **board, boardWidth, boardHeight;
int generation;
char *inFile, *outFile;

// for writing iterations
FILE *output;
int writeAlive = 1;
char writeDead = 'X';

int main(int argc, char **argv)
{
	initializeParameters(argc, argv);
	iterate(argc, argv);
	freeData();

	MPI_Finalize();
}

/****  FUNCTIONS  ****/
void iterate(int argc, char **argv)
{
	double totalTime;

	output = fopen(outFile, "w");

	int iters, err, totalProcs, myId;
	int workPerThread = -1;
	int extraWork = -1;

	MPI_Status status;

	int **nextStep = (int **)malloc(boardHeight * sizeof(int*));
	int i;
	for(i = 0; i<boardHeight; i++)
		nextStep[i] = (int *)malloc(boardWidth * sizeof(int));

	err = MPI_Init(&argc, &argv);
	err = MPI_Comm_size(MPI_COMM_WORLD, &totalProcs);
	err = MPI_Comm_rank(MPI_COMM_WORLD, &myId);

	/*
	 * TODO: Insert timing code
	 * 		MPI_Wtime??
	 */

	// iterate over multiple iterations
	for(iters = 0; iters<totalIterations; iters++)
	{
		int totalSize = boardHeight * boardWidth;

		/*
		 * Master process:
		 * 		Calculate how much work each process will do
		 *		Save it outside loop so it can be reused
		 */
		if(myId == 0)
		{
			// if work per process not found yet
			if(workPerThread == -1)
			{
				workPerThread = totalSize/totalProcs;
				extraWork = totalSize%totalProcs;
			}

			// send data to slave processes
			int procToSendTo;
			for(procToSendTo = 1; procToSendTo<totalProcs; procToSendTo++)
				err = MPI_Send(&workPerThread, 1, MPI_INT, procToSendTo, send_tag, MPI_COMM_WORLD);

			int workForMaster = workPerThread + extraWork;

			// do master's portion of work
			int workDone;
			int startCell = workPerThread*(totalProcs-1);
			int startRow = startCell/boardWidth;
			int startCol = startCell%boardWidth;
			for(workDone = 0; workDone<workForMaster; workDone++)
			{
				int newState = step_MPI(startRow, startCol);
				nextStep[startRow][startCol] = newState;

				startCol++;
				if(startCol == boardWidth)
				{
					startRow++;
					startCol = 0;
				}
			}

			// wait for rest of processes
			MPI_Barrier(MPI_COMM_WORLD);

			// switch pointers around
			int **temp = board;
			board = nextStep;
			nextStep = temp;

			generation++;

			if(iters != 0 && generation%printIterations == 0)
				writeGeneration(board);

		} // end master process work

		/*
		 * Slave process:
		 *		Recieve from master process
		 * 		All do same amount of work
		 *		Send data back to master to work out
		*/
		else
		{
			int amountOfWork, workDone;
			err = MPI_Recv(&amountOfWork, 1, MPI_INT, 0, send_tag, MPI_COMM_WORLD, &status);
			
			int startCell = amountOfWork*(myId-1);
			int startRow = startCell/boardWidth;
			int startCol = startCell%boardWidth;

			for(workDone = 0; workDone<amountOfWork; workDone++)
			{
				int nextState = step_MPI(startRow, startCol);
				nextStep[startRow][startCol] = nextState;

				startCol++;
				if(startCol == boardWidth)
				{
					startRow++;
					startCol = 0;
				}
			}

			MPI_Barrier(MPI_COMM_WORLD);

		} // end slave process work

	}// end iteration

	if(myId == 0)
	{
		// free old board
		for(i = 0; i<boardHeight; i++)
			free(nextStep[i]);
		free(nextStep);

		fclose(output);
	}
}

int step_MPI(int row, int col)
{
	int liveCells = getLiveNeighbors(row, col);

	// if dead
	if(board[row][col] == DEAD && liveCells == 3)
		return ALIVE;

	// if alive
	if(board[row][col] == ALIVE)
	{
		if(liveCells < 2 || liveCells > 3)
			return DEAD;
	}

	return board[row][col];
}

void step()
{
	generation++;
	int **nextStep = (int **)malloc(boardHeight * sizeof(int *));

	int i;
	for(i = 0; i<boardHeight; i++)
		nextStep[i] = (int *)malloc(boardWidth * sizeof(int));

	int r, c;
	for(r = 0; r<boardHeight; r++)
		for(c = 0; c<boardWidth; c++)
		{
			int liveCells = getLiveNeighbors(r, c);
			nextStep[r][c] = board[r][c];

			// if dead
			if(board[r][c] == 0 && liveCells == 3)
				nextStep[r][c] = 1;
			// if alive
			else
			{
				if(liveCells < 2 || liveCells > 3)
					nextStep[r][c] = 0;
			}
		}

	int **temp = nextStep;
	nextStep = board;
	board = temp;

	// free data
	for(i = 0; i<boardHeight; i++)
		free(nextStep[i]);
	free(nextStep);
}

int getLiveNeighbors(int r, int c)
{
	int alive = 0;
	int rTemp, cTemp;
	
	/* Check UP */
	// N
	rTemp = r-1;
	if(rTemp < 0)
		rTemp = boardHeight - 1;
	if(board[rTemp][c] == 1)
		alive++;

	/* Check RIGHT */
	// NE
	cTemp = c+1;
	if(cTemp > boardWidth-1)
		cTemp = 0;
	if(board[rTemp][cTemp] == 1)
		alive++;

	// E
	if(board[r][cTemp] == 1)
		alive++;

	/* Check DOWN */
	// SE
	rTemp = r+1;
	if(rTemp > boardHeight-1)
		rTemp = 0;
	if(board[rTemp][cTemp] == 1)
		alive++;

	// S
	if(board[rTemp][c] == 1)
		alive++;

	/* Check LEFT */
	// SW
	cTemp = c-1;
	if(cTemp < 0)
		cTemp = boardWidth-1;
	if(board[rTemp][cTemp] == 1)
		alive++;

	// W
	if(board[r][cTemp] == 1)
		alive++;

	/* Check UP (again) */
	// NW
	rTemp = r-1;
	if(rTemp < 0)
		rTemp = boardHeight-1;
	if(board[rTemp][cTemp] == 1)
		alive++;

	return alive;
}

/****  HELPERS  ****/

void initializeParameters(int argc, char **argv)
{
	if(argc != 5)
	{
		printf("Incorrect number of parameters\n");
		exit(1);
	}

	int size = strlen(argv[1]) + 5;
	inFile = (char *)malloc(size * sizeof(char));
	inFile = strcpy(inFile, argv[1]);
	inFile = strcat(inFile, ".txt");
	readMaze(inFile);

	size = strlen(argv[2]) + 5;
	outFile = (char *)malloc(size * sizeof(char));
	outFile = strcpy(outFile, argv[2]);
	outFile = strcat(outFile, ".txt");

	totalIterations = atoi(argv[3]);
	if(totalIterations < 0)
	{
		printf("Number of iterations must be greater than 0\n");
		exit(1);
	}

	printIterations = atoi(argv[4]);
	if(printIterations < 0)
	{
		printf("Number of iterations until board is printed must be greather than 0\n");
		exit(1);
	}
}

void readMaze(char *fileName)
{
	FILE *f = fopen(fileName, "r");
	if(f == NULL)
	{
		printf("File does not exist\n");
		fclose(f);
		exit(1);
	}

	fscanf(f, "%d %d", &boardHeight, &boardWidth);

	board = (int **)malloc(boardHeight * sizeof(int *));
	int i;
	for(i = 0; i<boardHeight; i++)
		board[i] = (int *)malloc(boardWidth * sizeof(int));

	int r, c;
	for(r = 0; r<boardHeight; r++)
		for(c = 0; c<boardWidth; c++)
		{
			int num;
			fscanf(f, "%d", &num);
			board[r][c] = num;
		}

	fclose(f);
	generation = 0;
}

void writeGeneration(int **currentBoard)
{
	int r, c;

	fprintf(output, "Generation %d\n", generation);
	fprintf(output, "********************************\n");

	for(r = 0; r<boardHeight; r++)
	{
		for(c = 0; c<boardWidth; c++)
		{
			int state = currentBoard[r][c];

			if(state == DEAD)
				fprintf(output, "X");
			else
				fprintf(output, "1");
		}

		fprintf(output, "\n");
	}
}

void printMaze()
{
	printf("Current State (Gen %d):\n", generation);

	int r, c;
	for(r = 0; r<boardHeight; r++)
	{
		for(c = 0; c<boardWidth; c++)
			printf("%d ", board[r][c]);
		printf("\n");
	}
}

void freeData()
{
	free(inFile);
	free(outFile);

	int i;
	for(i = 0; i<boardHeight; i++)
		free(board[i]);
	free(board);
}
