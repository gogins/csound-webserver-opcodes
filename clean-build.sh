#!/bin/bash
echo "Creating clean build..."
git submodule update --init --recursive
git submodule status --recursive
cd csound
git checkout csound6
git branch
git pull
cd ..
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
echo "Finished clean build."
