build-main: main.c
	mpicc -o main -std=gnu99 main.c -DITERATION_COUNT=3000 -DMSG_MIN_SIZE=1024 -DMSG_MAX_SIZE=102400 -DMSG_SIZE_STEP=1024

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

prepare-data:
	cat throughput-std-single-node.csv > throughput-single-node-all.csv
	cat throughput-buff-single-node.csv | tail -n +2 >> throughput-single-node-all.csv
	cat throughput-std-two-nodes.csv > throughput-two-nodes-all.csv
	cat throughput-buff-two-nodes.csv | tail -n +2 >> throughput-two-nodes-all.csv

	cat delay-std-single-node.csv > delay-single-node-all.csv
	cat delay-buff-single-node.csv | tail -n +2 >> delay-single-node-all.csv
	cat delay-std-two-nodes.csv > delay-two-nodes-all.csv
	cat delay-buff-two-nodes.csv | tail -n +2 >> delay-two-nodes-all.csv
