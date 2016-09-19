all: grid1 grid2

grid1: grid1.c
	mpicc -o HW2_1 grid1.c -lm


grid2: grid2.c
	mpicc -o HW2_2 grid2.c -lm

clean:
	$(RM) HW2_1 HW2_2
