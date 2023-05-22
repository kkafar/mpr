#!/usr/bin/env bash

# This script should be executed only from the directory it is placed in the repository.

# $1 -- path to data directory
# $2 -- path to mapper
# $3 -- path to reducer

data_dir="data-mock"
data_files=("data-1GB.txt" "data-5GB.txt" "data-10GB.txt")
rust_mapper="rust/mapper/target/release/mapper"
rust_reducer="rust/reducer/target/release/reducer"

# Workaround to avoid using shell buitin `time` cmd
time_cmd="$(which time)"

mkdir -p results
result_file="results/result-$(date +%y-%m-%d-%h-%M-%S).txt"

echo "time,mode,size" > "${result_file}"

for file in "${data_files[@]}"
do
  "${time_cmd}" --format="%E" --output "${result_file}" --append cat ${data_dir}/${file} | ${rust_mapper} | ${rust_reducer}
done


