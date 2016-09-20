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

	int row,max_color,color1,color2,flag;
	max_color = 2*n-2;
	flag=0;

	/*create communicator and groups*/
	/*creates diagonal groups*/
	if (world_rank == n-1){
		/*gets upper right corner of grid*/
		color1 = max_color;
		flag=1;
	}
	else if (world_rank == (n*n)-1-(n-1)){
		/*gets lower left corner of grid*/
		if (n!=3) color1 = max_color -1;
		else color1 = 2;
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
			color1 = world_rank % (n+1);
		}
		else if (row*(n+1)>world_rank){
			/*on left side of diagonal in lower half of grid*/
			color1 = max_color +1- (world_rank % (n+1));
		}
	}
	else {
		/*creates groups out of inner diagonals*/
		color1 = world_rank % (n+1);
	}
	
	row = world_rank/n;
	/*creates antidiagonal groups*/
	if (world_rank ==0){
		color2 = 0;
	}
	else if(world_rank == n*n-1){
		color2 = max_color;
	}
	else{
		if((row+1)*(n-1)<world_rank){
			/*upper left triangle of grid*/	
			color2=world_rank%(n-1);
		}
		else if ((row+1)*(n-1)>world_rank){
			/*lower right triangle of grid*/
			color2=max_color-(world_rank%(n-1));
		}
		else if ((row+1)*(n-1)==world_rank){
			/*antidiagonal*/
			color2=max_color/2;
		}
	}
	/*printf("color1 %d world rank %d world size %d\n", color1, world_rank, world_size);
	
	  printf("color2 %d world rank %d world size %d\n", color2, world_rank, world_size);
	*/
	MPI_Comm diag1,diag2;
	MPI_Comm_split(MPI_COMM_WORLD, color1, world_rank, &diag1);
	MPI_Comm_split(MPI_COMM_WORLD, color2, world_rank, &diag2);
	int diag_rank1, diag_size1, diag_rank2, diag_size2;
	MPI_Comm_rank(diag1, &diag_rank1);
	MPI_Comm_size(diag1, &diag_size1);
	MPI_Comm_rank(diag2, &diag_rank2);
	MPI_Comm_size(diag2, &diag_size2);

	/*printf("color1 %d diag rank %d/%d world rank %d\n", color1, diag_rank1, diag_size1,world_rank);
	printf("color2 %d diag rank %d/%d world rank %d\n", color2, diag_rank2, diag_size2,world_rank);
	*/
	int token;
	
	if (diag_rank1 == 0 && flag != 1){
		token = world_rank;
		MPI_Send(&token, 1, MPI_INT, diag_rank1+1, 0, diag1);
		printf("diagonal %d sent token\n",world_rank);
		MPI_Recv(&token, 1, MPI_INT, diag_rank1+1, 0, diag1, MPI_STATUS_IGNORE);	
		printf("diagonal %d received token from %d. back at start\n",world_rank,token);
	}
	else if (flag != 1){
		MPI_Recv(&token, 1, MPI_INT, diag_rank1-1, 0, diag1, MPI_STATUS_IGNORE);
		printf("diagonal %d received token from %d\n",world_rank, token);
		token=world_rank;
		if (diag_rank1+1 != diag_size1){
			MPI_Send(&token, 1, MPI_INT, diag_rank1+1, 0, diag1);
			printf("diagonal %d sent token\n",world_rank);
			
			MPI_Recv(&token, 1, MPI_INT, diag_rank1+1, 0, diag1, MPI_STATUS_IGNORE);
			printf("diagonal if %d received token from %d\n",world_rank,token);
			
			token=world_rank;
		}
		MPI_Send(&token, 1, MPI_INT, diag_rank1-1, 0, diag1);
		printf("diagonal %d sent token back up diagonal\n", world_rank);
	}
	else{
		MPI_Send(&token, 1, MPI_INT, diag_rank1, 0, diag1);
		printf("diagonal sent token to self %d\n",world_rank);
		MPI_Recv(&token, 1, MPI_INT, diag_rank1, 0, diag1, MPI_STATUS_IGNORE);
		printf("diagonal received token from self %d\n",world_rank);
	}

	if (diag_rank2 == 0 && flag != 1){
		token = world_rank;
		MPI_Send(&token, 1, MPI_INT, diag_rank2+1, 0, diag2);
		printf("antidiagonal %d sent token\n",world_rank);
		MPI_Recv(&token, 1, MPI_INT, diag_rank2+1, 0, diag2, MPI_STATUS_IGNORE);	
		printf("antidiagonal %d received token from %d. back at start\n",world_rank,token);
	}
	else if (flag != 1){
		MPI_Recv(&token, 1, MPI_INT, diag_rank2-1, 0, diag2, MPI_STATUS_IGNORE);
		printf("antidiagonal %d received token from %d\n",world_rank, token);
		token=world_rank;
		if (diag_rank2+1 != diag_size2){
			MPI_Send(&token, 1, MPI_INT, diag_rank2+1, 0, diag2);
			printf("antidiagonal %d sent token\n",world_rank);
			
			MPI_Recv(&token, 1, MPI_INT, diag_rank2+1, 0, diag2, MPI_STATUS_IGNORE);
			printf("antidiagonal if %d received token from %d\n",world_rank,token);
			
			token=world_rank;
		}
		MPI_Send(&token, 1, MPI_INT, diag_rank2-1, 0, diag2);
		printf("antidiagonal %d sent token back up\n", world_rank);
	}
	else{
		MPI_Send(&token, 1, MPI_INT, diag_rank2, 0, diag2);
		printf("antidiagonal sent token to self %d\n",world_rank);
		MPI_Recv(&token, 1, MPI_INT, diag_rank2, 0, diag2, MPI_STATUS_IGNORE);
		printf("antidiagonal received token from self %d\n",world_rank);
	}
	
	MPI_Finalize();
}
