build-main: main.c
	mpicc -o main -std=gnu99 main.c -DITERATION_COUNT=2500 -DMSG_MIN_SIZE=1024 -DMSG_MAX_SIZE=40960 -DMSG_SIZE_STEP=1024

run-main: build-main
	mpiexec -machinefile ./allnodes -np 2 ./main

run-main-single-node: build-main
	mpiexec -machinefile ./single-node -np 2 ./main
	mv delay-std.csv delay-std-single-node.csv
	mv throughput-std.csv throughput-std-single-node.csv
	mv delay-buff.csv delay-buff-single-node.csv
	mv throughput-buff.csv throughput-buff-single-node.csv

run-main-two-nodes: build-main
	mpiexec -machinefile ./two-nodes -np 2 ./main
	mv delay-std.csv delay-std-two-nodes.csv
	mv throughput-std.csv throughput-std-two-nodes.csv
	mv delay-buff.csv delay-buff-two-nodes.csv
	mv throughput-buff.csv throughput-buff-two-nodes.csv
