#!/bin/bash
# Build script for Windows (Git Bash)

# Find MinGW GCC
if command -v g++ &> /dev/null; then
    CXX_COMPILER=$(which g++)
    echo "Found GCC at: $CXX_COMPILER"
else
    echo "ERROR: g++ not found in PATH"
    exit 1
fi

# Clean build directory
rm -rf build
mkdir -p build
cd build

# Configure with explicit compiler
cmake -G "Ninja" \
    -DCMAKE_CXX_COMPILER="$CXX_COMPILER" \
    -DCMAKE_BUILD_TYPE=Release \
    ..

# Build
if [ $? -eq 0 ]; then
    echo "Configuration successful. Building..."
    ninja

    if [ $? -eq 0 ]; then
        echo ""
        echo "==============================================="
        echo "BUILD SUCCESSFUL!"
        echo "==============================================="
        echo ""
        echo "Run server: ./build/drone_server.exe 8080"
        echo "Run client: ./build/client/test_client.exe localhost 8080 --rate 100"
    else
        echo "BUILD FAILED"
        exit 1
    fi
else
    echo "CMake configuration failed"
    exit 1
fi
