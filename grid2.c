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
	
	row = world_rank/n;

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

	/* color 1 are the groups on the diagonals
	 * color 2 are the groups on the antidiagonals
	 * diag1 is diagonal communicator
	 * diag2 is antidiagonal communicator
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
	
	/* tokenx[0]: starting world rank 
	 * tokenx[1]: starting direction of communicator (0 diagonal, 1 antidiagonal)
	 * tokenx[2]: is this communicator sequence complete (0 no, 1 yes)
	 * tokenx[3]: last processor to touch token
	 */
	//int token1[4],token2[4],token[4];
	int token1[3],token2[3],token[3];
	int t_size=3*sizeof(int);
	/*
	 * tflagc checks to see if there is a message on the global communicator
	 * tflaga checks to see if there is a message on the antidiagonal communicator
	 * tflagd checks to see if there is a message on the diagonal communicator
	 */
	int tflagc=0,tflaga=0,tflagd=0;
	MPI_Status status;
	flag =0;

	token[0]=-1;
	token[1]=-1;
	token[2]=-1;
	//token[3]=-1;


	/* pass token alternating communicators until it receives rank 0
	 * of starting communicator
	 */
	//printf("created token and flags %d\n",world_rank);
	if (diag_rank1 == 0 && diag_rank2 == 0){
		/*on the diagonal communicator*/
		printf("in diag rank 0 world rank %d\n",world_rank);
		token1[0]=world_rank;
		token1[1]=0;
		token1[2]=0;
		//token1[3]=world_rank;
		//printf("token1 values for %d 0: %d 1: %d 2: %d\n",
		//		world_rank, token1[0],token1[1],token1[2]);
		MPI_Send(&token1, 3*sizeof(int), MPI_INT, 
				(diag_rank1+1)%diag_size1, 0, diag1);
		printf("%d started token on diagonal\n",world_rank);
		/*on the antidiagonal communicator*/
		token2[0]=world_rank;
		token2[1]=1;
		token2[2]=0;
		//token2[3]=world_rank;
		//printf("token2 values for %d 0: %d 1: %d 2: %d\n",
		//		world_rank, token2[0],token2[1],token2[2]);
		MPI_Send(&token2, 3*sizeof(int), MPI_INT, 
				(diag_rank2+1)%diag_size2, 0, diag2);
		printf("%d started token on antidiagonal\n",world_rank);
		
		while (flag!=n*2-1){
			while(!tflagc&&!tflaga&&!tflagd){
				//printf("%d about to start Iprobe\n",world_rank);
				MPI_Iprobe((diag_rank1-1)%diag_size1,0,
						diag1,&tflagd,&status);
				//printf("%d about to start iprobe2\n",world_rank);
				MPI_Iprobe((diag_rank2-1)%diag_size2,0,
						diag2,&tflaga,&status);
				//printf("%d about to start iprobe3\n",world_rank);
				MPI_Iprobe((world_rank-1)%world_size,0,
						MPI_COMM_WORLD,&tflagc,&status);
				//printf("%d finished iprobes\n",world_rank);
				//printf("tflagc: %d tflaga: %d tflagd: %d\n",
				//		tflagc,tflaga,tflagd);
			}

			//printf("%d tflagc: %d tflaga: %d tflagd: %d\n",
			//		world_rank,tflagc,tflaga,tflagd);

			if (tflagc){
				//printf("%d flag: %d before increment\n",world_rank,flag);
				flag++;
				//printf("%d flag: %d after increment\n",world_rank,flag);
				tflagc=0;

				MPI_Recv(&token, 3*sizeof(int), MPI_INT, 
						(world_rank-1)%world_size, 0, MPI_COMM_WORLD, &status);
				//printf("%d received finished token from %d\n",world_rank,token[3]);
				//token[3]=world_rank;
				if (token[0]!=world_rank) MPI_Send(&token, 3*sizeof(int), 
						MPI_INT, (world_rank+1)%world_size, 0, MPI_COMM_WORLD);
				else printf("%d token for %d has finished\n",token[1], world_rank);
			}
			if (tflaga){
				tflaga=0;

				if(token[0]!=world_rank || token[1]!=1){
					MPI_Recv(&token, 3*sizeof(int), MPI_INT, 
							(diag_rank2-1)%diag_size2, 0, diag2, &status);
					//printf("%d received token from antidiagonal for %d from %d\n",
					//		world_rank, token[0],token[3]);
					//token[3]=world_rank;
					MPI_Send(&token, 3*sizeof(int), MPI_INT, 
							(diag_rank1+1)%diag_size1, 0, diag1);
					//printf("%d sent token along diagonal\n",world_rank);
				}
				else
				{
					MPI_Recv(&token, 3*sizeof(int), MPI_INT, 
							(diag_rank2-1)%diag_size2, 0, diag2, &status);
					//printf("%d received finished token from antidiagonal for %d from %d\n",
					//		world_rank, token[0],token[3]);
					token[2]=1;
					//token[3]=world_rank;
					MPI_Send(&token, 3*sizeof(int), MPI_INT, 
							(world_rank+1)%world_size, 0, MPI_COMM_WORLD);
					//printf("%d sent token along world finished\n",world_rank);
				}
			}
			if (tflagd){
				tflagd=0;

				if(token[0]!=world_rank || token[1]!=0){
					MPI_Recv(&token, 3*sizeof(int), MPI_INT, 
							(diag_rank1-1)%diag_size1, 0, diag1, &status);
					//printf("%d received token from diagonal for %d from %d\n",
					//		world_rank, token[0], token[3]);
					//token[3]=world_rank;
					MPI_Send(&token, 3*sizeof(int), MPI_INT, 
							(diag_rank2+1)%diag_size2, 0, diag2);
					//printf("%d sent token along antidiagonal\n",world_rank);
				}
				else{
					MPI_Recv(&token, 3*sizeof(int), MPI_INT, 
							(diag_rank1-1)%diag_size1, 0, diag1, &status);
					//printf("%d received finished token from diagonal for %d from %d\n",
					//		world_rank, token[0],token[3]);
					token[2]=1;
					//token[3]=world_rank;
					MPI_Send(&token, 3*sizeof(int), MPI_INT, 
							(world_rank+1)%world_size, 0, MPI_COMM_WORLD);
					//printf("%d sent finished token along world\n",world_rank);
				}
			}
		}
	}
	else{
		printf("%d is in else\n",world_rank);
		printf("%d will stop when flag=%d\n",world_rank,6+(n-2)*2);
		while (flag!=n*2-1){
			while(!tflagc&&!tflaga&&!tflagd){
				//printf("%d about to start Iprobe\n",world_rank);
				MPI_Iprobe((diag_rank1-1)%diag_size1,
						0, diag1,&tflagd,&status);
				//printf("%d about to start iprobe2\n",world_rank);
				MPI_Iprobe((diag_rank2-1)%diag_size2,
						0,diag2,&tflaga,&status);
				//printf("%d about to start iprobe3\n",world_rank);
				MPI_Iprobe((world_rank-1)%world_size,
						0,MPI_COMM_WORLD,&tflagc,&status);
				//printf("%d finished iprobes\n",world_rank);
				//printf("tflagc: %d tflaga: %d tflagd: %d\n",
				//		tflagc,tflaga,tflagd);
			}
			//printf("%d out of iprobe loop about to process a communicator\n",
			//		world_rank);
			//printf("%d tflagc: %d tflaga: %d tflagd: %d\n",
			//		world_rank,tflagc,tflaga,tflagd);
			if (tflagc){
				/*
				 * global communicator keeps track of how many of 
				 * the communicators are finished. Once all of the
				 * communicators are finished end the loop
				 */
				//printf("%d received token that finished for %d on diagonal %d\n",
				//		world_rank, token[0], token[1]);
				printf("%d flag: %d before increment\n",world_rank,flag);
				flag++;
				printf("%d flag: %d after increment\n",world_rank,flag);
				tflagc=0;
				MPI_Recv(&token, 3*sizeof(int), MPI_INT, 
						(world_rank-1)%world_size, 0, MPI_COMM_WORLD, &status);
				//printf("%d received finished token from %d\n",world_rank,token[3]);
				//token[3]=world_rank;
				MPI_Send(&token, 3*sizeof(int), MPI_INT, 
						(world_rank+1)%world_size, 0, MPI_COMM_WORLD);
			}
			if (tflaga){
				/*
				 * token received on antidiagonal.
				 * send token along diagonal
				 */
				tflaga=0;
				MPI_Recv(&token, 3*sizeof(int), MPI_INT, 
						(diag_rank2-1)%diag_size2, 0, diag2, &status);
				//printf("%d received token from antidiagonal for %d from %d\n",
				//		world_rank, token[0], token[3]);
				//token[3]=world_rank;
				MPI_Send(&token, 3*sizeof(int), MPI_INT, 
						(diag_rank1+1)%diag_size1, 0, diag1);
				//printf("%d sent token along diagonal\n",world_rank);
			}
			if (tflagd){
				/*
				 * token received on diagonal.
				 * send token along antidiagonal
				 */
				tflagd=0;
				MPI_Recv(&token, 3*sizeof(int), MPI_INT, 
						(diag_rank1-1)%diag_size1, 0, diag1, &status);
				//printf("%d received token from diagonal for %d from %d\n",
				//		world_rank, token[0],token[3]);
				//token[3]=world_rank;
				MPI_Send(&token, 3*sizeof(int), MPI_INT, 
						(diag_rank2+1)%diag_size2, 0, diag2);
				//printf("%d sent token along antidiagonal\n",world_rank);
			}
		}
	}

	printf("%d is finished\n",world_rank);

	MPI_Finalize();
}
