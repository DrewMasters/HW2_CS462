#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv){
	if (argc !=2) {
		printf("./grid desired_dimension_of_grid\n");
		exit(1);
	}
	
	int dim;
	dim = atoi(argv[1]);
	
}
