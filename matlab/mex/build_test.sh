#!/bin/bash
# Build test script for DUCC0 MATLAB MEX interface
# This script tests if the MEX files can be built

set -e

echo "========================================"
echo "DUCC0 MATLAB MEX Interface - Build Test"
echo "========================================"
echo ""

# Check if MATLAB is available
if ! command -v matlab &> /dev/null; then
    echo "ERROR: MATLAB not found in PATH"
    echo "Please add MATLAB to PATH or set MATLAB_ROOT"
    exit 1
fi

echo "MATLAB found"
echo ""

# Check if CMake is available
if ! command -v cmake &> /dev/null; then
    echo "ERROR: CMake not found in PATH"
    echo "Please install CMake or add it to PATH"
    exit 1
fi

echo "CMake found"
echo ""

# Create build directory
mkdir -p build
cd build

# Run CMake
echo "Running CMake..."
cmake ..
if [ $? -ne 0 ]; then
    echo "ERROR: CMake configuration failed"
    exit 1
fi

echo "CMake configuration succeeded"
echo ""

# Build
echo "Building MEX files..."
cmake --build .
if [ $? -ne 0 ]; then
    echo "ERROR: Build failed"
    exit 1
fi

echo "Build succeeded"
echo ""

# Check if MEX files were created
echo "Checking for MEX files..."
if ls *.mex* 1> /dev/null 2>&1; then
    echo "MEX files found:"
    ls -lh *.mex*
else
    echo "WARNING: No MEX files found in build directory"
    exit 1
fi

echo ""
echo "========================================"
echo "Build test completed successfully"
echo "========================================"
echo ""

cd ..

