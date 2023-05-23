#!/usr/bin/env bash

root_dir=$(pwd)
cd rust/mapper
cargo build --release
cargo install --root ${root_dir} --path .

cd ../reducer
cargo build --release
cargo install --root ${root_dir} --path .
cd ${root_dir}

mv bin/* .

