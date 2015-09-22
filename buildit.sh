#!/usr/bin/env bash

cd /sim
mkdir build
cd build
cmake .. -G Ninja
ninja
ninja install
