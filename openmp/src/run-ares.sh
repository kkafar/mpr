#/bin/bash -l

module add gcc/10.3.0


# Sync experiment
sbatch ./run.sh -t sync -s 10 -p 1 -P 1 -b 1 -B 100 -I 1

