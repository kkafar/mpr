#!/bin/bash -l

echo "Adding plgrid/tools/openmpi module"
module add .plgrid plgrid/tools/openmpi

# Do not process data, do not archive, do not compile
make all

if [[ $? -ne 0 ]]
then
  exit 1
fi

sbatch ./run.sh -C -D -Z -S "1 2 3 4 5"
sbatch ./run.sh -C -D -Z -S "6 7 8 9 10"
sbatch ./run.sh -C -D -Z -s "1 2 3 4 5"
sbatch ./run.sh -C -D -Z -s "6 7 8 9 10"

