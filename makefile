all: grid.c
	mpicc -o HW2 grid.c -lm
clean:
	$(RM) HW2
