build-main: main.c
	mpicc -o main -std=gnu99 main.c -DITERATION_COUNT=1000 -DCONFIG_CODE_NAME="EXP1"

run-main: build-main
	mpiexec -machinefile ./allnodes -np 2 main


