main: main.c
	mpicc -o main -std=gnu99 main.c 

run: build-main
	mpiexec -machinefile ./allnodes -np 2 ./main 1000000

