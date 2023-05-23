#!/usr/bin/env bash

sizes=("1" "5" "10")

mode="unknown"
mapper="rust/mapper/target/release/mapper"
reducer="rust/reducer/target/release/reducer"

bucket_url="s3://bucket-mpr"
input_file="${bucket_url}/data-1GB.txt"
output_dir="results"


if [[ $# -gt 0 ]]
then
  mode="$1"  
fi

if [[ $# -gt 1 ]]
then
  mapper="$2"  
fi

if [[ $# -gt 2 ]]
then
  reducer="$3"  
fi

if [[ $# -gt 3 ]]
then
  input_file="$4"  
fi


# Workaround to avoid using shell buitin `time` cmd
time_cmd="$(which time)"
${time_cmd} --format "%e" hadoop jar /usr/lib/hadoop/hadoop-streaming.jar \
  -files "${mapper}","${reducer}" \
  -mapper "${mapper}" \ 
  -reducer "${reducer}" \
  -input "${input_file}" \
  -output "${output_dir}"

hdfs dfs -get ${output_dir}
