#!/usr/bin/env bash

# Rust installation
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
source $HOME/.cargo/env

# Package manager update
sudo yum update -y

# Git installation
sudo yum groupinstall "Development Tools"



