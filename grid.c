#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv){
	if (argc !=2) {
		printf("./grid desired_dimension_of_grid\n");
		exit(1);
	}
	
	int dim, world_size, world_rank;;
	/*read in grid dimension specified by user*/
	dim = atoi(argv[1]);
	
	/*initialize MPI*/
	MPI_Init(NULL,NULL);

	/*get world size and rank*/
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
	
	/*create communicator and groups*/
	
	/*pass token*/

	MPI_Finalize();
}
