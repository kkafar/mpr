.PHONY: build run cleanall clean clean-data

all: build

build: main.c
	mpicc -DOPAL_STDC_HEADERS=0 -fast -o main -std=c99 main.c 

run: build
	mpiexec -machinefile ./allnodes -np 2 ./main 1000000

cleanall: clean clean-data
	@echo "Cleaning all build artifacts & output files"

clean:
	rm -vfr main

clean-data:
	rm -vfr data
	

