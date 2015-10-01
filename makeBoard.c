#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define SIZE 1000

int main(int argc, char **argv)
{
	int r, c;

	FILE *f = fopen("initialState.txt", "w");

	srand(time(NULL));

	fprintf("%d %d\n", SIZE, SIZE);
	for(r = 0; r<SIZE; r++)
	{
		for(c = 0; c<SIZE, c++)
		{
			int state = rand() % 2;
			fprintf(f, "%d", state);
		}
		fprintf(f, "\n");
	}

	fclose(f);

	return 0;
}
