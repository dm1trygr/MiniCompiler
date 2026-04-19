#!/bin/bash

LLVM_DIR="../llvm-project"

if [ ! -d "$LLVM_DIR" ]; then
    echo "LLVM not found, cloning..."
    git clone --depth 1 --branch llvmorg-18.1.0 https://github.com/llvm/llvm-project.git "$LLVM_DIR"
fi

if [ ! -d "$LLVM_DIR/build" ]; then
    echo "Building LLVM..."
    cmake -S "$LLVM_DIR/llvm" -B "$LLVM_DIR/build" \
        -DCMAKE_BUILD_TYPE=Release \
        -DLLVM_ENABLE_PROJECTS="" \
        -DLLVM_TARGETS_TO_BUILD="X86" \
        -DLLVM_BUILD_TOOLS=OFF \
        -DLLVM_BUILD_EXAMPLES=OFF \
        -DLLVM_INCLUDE_TESTS=OFF \
        -DLLVM_INCLUDE_BENCHMARKS=OFF
    cmake --build "$LLVM_DIR/build" -j$(nproc)
fi

mkdir -p build
if [ ! -d build ]; then
    echo "Can't create build directory"
    exit 1
fi

cd build

cmake .. -DLLVM_DIR="$(realpath ../../llvm-project/build/lib/cmake/llvm)"
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
