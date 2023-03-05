build-main: main.c
	mpicc -o main -std=gnu99 main.c -DITERATION_COUNT=10000 -DMSG_MIN_SIZE=1024 -DMSG_MAX_SIZE=40960 -DMSG_SIZE_STEP=1024

run-main: build-main
	mpiexec -machinefile ./allnodes -np 2 ./main

run-main-single: build-main
	mpiexec -machinefile ./single-node -np 2 ./main
	mv delay-std.csv delay-single-node.csv
	mv throughput-std.csv throughput-single-node.csv

run-main-two-nodes: build-main
	mpiexec -machinefile ./two-nodes -np 2 ./main
	mv delay-std.csv delay-two-nodes.csv
	mv throughput-std.csv throughput-two-nodes.csv


