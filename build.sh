#!/bin/bash

echo "========================================"
echo "SeshNx Reactor - Build Script"
echo "========================================"

BUILD_TYPE=${1:-Release}

echo "Build Type: $BUILD_TYPE"
echo ""

# Create build directory
mkdir -p build

# Configure CMake
echo "[1/2] Configuring CMake..."
cmake -B build -S . -DCMAKE_BUILD_TYPE=$BUILD_TYPE
if [ $? -ne 0 ]; then
    echo "CMake configuration failed!"
    exit 1
fi

# Build
echo ""
echo "[2/2] Building..."
cmake --build build --config $BUILD_TYPE
if [ $? -ne 0 ]; then
    echo "Build failed!"
    exit 1
fi

echo ""
echo "========================================"
echo "Build Successful!"
echo "========================================"
echo ""
echo "Output locations:"
echo "  VST3:       build/Reactor_artefacts/$BUILD_TYPE/VST3/"
if [[ "$OSTYPE" == "darwin"* ]]; then
    echo "  AU:         build/Reactor_artefacts/$BUILD_TYPE/AU/"
fi
echo "  Standalone: build/Reactor_artefacts/$BUILD_TYPE/Standalone/"
echo ""
