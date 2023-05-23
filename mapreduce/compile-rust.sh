#!/usr/bin/env bash

root_dir=$(pwd)
cd rust/mapper
cargo build --release
cd ../reducer
cargo build --release
cd ${root_dir}
