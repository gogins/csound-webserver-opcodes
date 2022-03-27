#!/bin/bash
echo "Creating completely fresh build..."
git submodule update --init --recursive
git submodule status --recursive
rm -rf build/*
mkdir -p build
cd build
echo "Configuring..."
cmake .. -DCMAKE_PREFIX_PATH=/usr/local:/usr
echo "Building..."
make
echo "Packaging..."
cpack -V
ls -ll
echo "Finished completely fresh build."
