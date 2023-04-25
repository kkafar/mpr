#!/bin/bash

rootdir="$(pwd)"

n_series=10
n_elems=16777216 # 128 MB
n_buckets=32

data_dir="data/raw"
mkdir -p ${data_dir}
mkdir -p "data-arch/prng"

./prng ${n_elems} ${n_buckets} ${n_series} | tee "${data_dir}/prng_size_${n_elems}_buckets_${n_buckets}.csv"

cp "${data_dir}/prng_size_${n_elems}_buckets_${n_buckets}.csv" "data-arch/prng/final-prng.csv"

