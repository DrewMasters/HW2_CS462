#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int main(int argc, char **argv){
	int world_size, world_rank,n;
	
	/*initialize MPI*/
	MPI_Init(NULL,NULL);

	/*get world size and rank*/
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
	n = (int) sqrt(world_size);
	if (sqrt(world_size)-n!=0){
		MPI_Finalize();
		exit(1);
	}

	int max_color,color;
	max_color = 2*n-2;

	/*create communicator and groups*/
	if (world_rank == n-1){
		color = max_color;
		printf("color %d world rank %d world size %d if\n", color,world_rank, world_size);
	}
	else if (world_rank == (n*n)-1-(n-1)){
		if (n!=3) color = max_color -1;
		else color = 2;
		printf("color %d world rank %d world size %d (n*n)-n-2 %d\n", color,world_rank, world_size,(n*n)-n-2);
	}
	else if (n==5){/*need to fix this can only handle n=4 and n=5*/
		if (world_rank==15||world_rank==21) color = max_color -2;
		else color=world_rank %(n+1);
	}
	else {
		color = world_rank % (n+1);
		printf("color %d world rank %d world size %d else\n", color,world_rank, world_size);
	}
	/*printf("color %d world rank %d world size %d\n", color, world_rank, world_size);
	*/
	MPI_Comm diag;
	MPI_Comm_split(MPI_COMM_WORLD, color, world_rank, &diag);
	int diag_rank, diag_size;
	MPI_Comm_rank(diag, &diag_rank);
	MPI_Comm_rank(diag, &diag_size);
	
	printf("diag rank %d world rank %d color %d\n", diag_rank, world_rank, color);

	/*pass token*/

	MPI_Finalize();
}
