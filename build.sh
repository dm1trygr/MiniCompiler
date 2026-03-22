#!/bin/bash

mkdir -p build
if [ ! -d build ]; then
    echo "Can't create build directory"
    exit 1
fi

cd build

cmake ..
if [[ ! $? -eq 0 ]]
then
  echo "CMake error"
  exit 1
fi

make
if [[ ! $? -eq 0 ]]
then
  echo "Compilation error"
  exit 1
fi

if [ -f compiler ]; then
    mv compiler ..
else
    echo "Error: executable "compiler" not created"
    exit 1
fi

cd ..
rm -r build
