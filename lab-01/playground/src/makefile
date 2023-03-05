build-main: main.c
	mpicc -o main -std=gnu99 main.c -DITERATION_COUNT=10000 -DCONFIG_CODE=1

run-main: build-main
	mpiexec -machinefile ./allnodes -np 2 ./main


