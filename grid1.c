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

	int row,max_color,color,flag;
	max_color = 2*n-2;
	flag=0;

	/*create communicator and groups*/
	if (world_rank == n-1){
		/*gets upper right corner of grid*/
		color = max_color;
		flag=1;
	}
	else if (world_rank == (n*n)-1-(n-1)){
		/*gets lower left corner of grid*/
		if (n!=3) color = max_color -1;
		else color = 2;
		flag=1;
	}
	else if ((world_rank % (n+1))>=3 && (world_rank % (n+1))<(n-1)){
		row = world_rank/n;
		/*breaks up diagonals that wrap around the grid
		 * into two communicators instead of one that wraps around
		 * depending on which side of the diagonal it is on
		 */
		if (row*(n+1)<world_rank){
			/*on right side of diagonal in upper half of grid*/
			color = world_rank % (n+1);
		}
		else if (row*(n+1)>world_rank){
			/*on left side of diagonal in lower half of grid*/
			color = max_color +1- (world_rank % (n+1));
		}
	}
	else {
		/*creates groups out of inner diagonals*/
		color = world_rank % (n+1);
	}
	/*printf("color %d world rank %d world size %d\n", color, world_rank, world_size);
	*/
	MPI_Comm diag;
	MPI_Comm_split(MPI_COMM_WORLD, color, world_rank, &diag);
	int diag_rank, diag_size;
	MPI_Comm_rank(diag, &diag_rank);
	MPI_Comm_size(diag, &diag_size);
	
	printf("diag rank %d/%d world rank %d color %d\n", diag_rank, diag_size,world_rank, color);
	
	int token;

	if (diag_rank == 0 && flag != 1){
		token = world_rank;
		MPI_Send(&token, 1, MPI_INT, diag_rank+1, 0, diag);
		printf("%d sent token\n",world_rank);
		MPI_Recv(&token, 1, MPI_INT, diag_rank+1, 0, diag, MPI_STATUS_IGNORE);	
		printf("token back at starting point\n");
	}
	else if (flag != 1){
		MPI_Recv(&token, 1, MPI_INT, diag_rank-1, 0, diag, MPI_STATUS_IGNORE);
		printf("%d received token from %d\n",world_rank, token);
		if (diag_rank+1 != diag_size){
			token=world_rank;
			MPI_Send(&token, 1, MPI_INT, diag_rank+1, 0, diag);
			printf("%d sent token\n",world_rank);
			MPI_Recv(&token, 1, MPI_INT, diag_rank+1, 0, diag, MPI_STATUS_IGNORE);
			printf("%d received token from %d\n",world_rank,token);
		}
		MPI_Send(&token, 1, MPI_INT, diag_rank-1, 0, diag);
		printf("%d sent token back up diagonal\n", world_rank);
	}
	else{
		MPI_Send(&token, 1, MPI_INT, diag_rank, 0, diag);
		printf("sent token to self %d\n",world_rank);
		MPI_Recv(&token, 1, MPI_INT, diag_rank, 0, diag, MPI_STATUS_IGNORE);
		printf("received token from self %d\n",world_rank);
	}
	
	MPI_Finalize();
}
