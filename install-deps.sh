#!/bin/bash

apt update -y

apt install -y vim build-essential curl

curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
