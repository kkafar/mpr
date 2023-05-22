#!/usr/bin/env bash

# This script should be executed only from the directory it is placed in the repository.

# $1 -- path to data directory
# $2 -- path to mapper
# $3 -- path to reducer

data_dir="data-mock"
data_files=("data-1GB.txt" "data-5GB.txt" "data-10GB.txt")
rust_mapper="rust/mapper/target/release/mapper"
rust_reducer="rust/reducer/target/release/reducer"
mode="unknown"
sizes=("1" "5" "10")

if [[ $# -gt 0 ]]
then
  data_dir="$1"  
fi

if [[ $# -gt 1 ]]
then
  rust_mapper="$2"  
fi

if [[ $# -gt 2 ]]
then
  rust_reducer="$3"  
fi

if [[ $# -gt 3 ]]
then
  mode="$4"  
fi

# Workaround to avoid using shell buitin `time` cmd
time_cmd="$(which time)"

mkdir -p results/tmp
result_file="results/result-$(date +%y-%m-%d-%H-%M-%S).txt"
tmp_file="results/tmp/tmp-${result-file}"

echo "time,mode,size" > "${result_file}"

for i in $(seq 0 1 "$(( ${#data_files[@]} - 1))")
do
  file="${data_files[${i}]}"
  size="${sizes[${i}]}"
  # echo "Running for file ${file}, size ${size}"
  "${time_cmd}" --format="%E" --output ${tmp_file} cat ${data_dir}/${file} | ${rust_mapper} | ${rust_reducer} &> /dev/null
  cat ${tmp_file} | awk -v mode="${mode}" -v size="${size}" -F ',' '{ print $0 "," mode "," size }' >> ${result_file}
done

rm -fr "results/tmp/"

