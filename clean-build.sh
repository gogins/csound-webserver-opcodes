#!/bin/bash
clear
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
export BOOST_ROOT=/opt/homebrew/Cellar/boost/1.79.0_1
cmake .. -DCMAKE_PREFIX_PATH=/usr/local:/usr
echo "Building..."
make
cd ..
echo "Documenting..."
doxygen
cd build
echo "Packaging..."
cpack
ls -ll
echo "Finished clean build."
