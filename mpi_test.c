#include "mpi.h"
#include <stdio.h>

int main(){
	int numtasks, rank, dest, source, rc, count, tag=1;

	MPI_Init(NULL,NULL);
	MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	int number = 0;
	int x = 2;
	while (x>0){
		if (rank == number) {
			dest = (rank+1)%numtasks;
			number = dest;
			rc = MPI_Ssend(&number, 1, MPI_INT, dest, tag, MPI_COMM_WORLD);
			printf("Processor %d: sent to Process %d , number = %d\n", rank, dest, number);
		}
		else {
			source = rank-1;
			if (source < 0){
				source = numtasks - 1;
			}
			rc = MPI_Recv(&number, 1, MPI_INT, source, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			printf("Processor %d: recieved from processor %d, number= %d \n", rank, source, number);
		}
		x--;
	}

}
