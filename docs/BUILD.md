# Build Instructions

**Repository:** `https://github.com/usuued/tech19_test`

---

## Prerequisites

### Operating System
- **Ubuntu 20.04 LTS** or later (recommended)
- **Debian 10+** or other Linux distributions with glibc 2.27+
- **WSL2** on Windows (Ubuntu distribution)

### Required Tools
```bash
# Check versions
gcc --version      # GCC 9+ required
g++ --version      # G++ 9+ required
cmake --version    # CMake 3.15+ required
make --version     # GNU Make 4+
```

### Installing Prerequisites (Ubuntu/Debian)
```bash
sudo apt update
sudo apt install -y build-essential cmake git
```

### Optional Tools
```bash
# For memory leak detection
sudo apt install -y valgrind

# For unit tests
# (Built-in simple framework, no external dependencies)
```

---

## Quick Start

### 1. Clone Repository
```bash
git clone https://github.com/usuued/tech19_test.git
cd tech19_test
```

### 2. Build
```bash
mkdir build
cd build
cmake ..
make -j$(nproc)
```

### 3. Run Server
```bash
./drone_server 8080
```

### 4. Run Test Client (in another terminal)
```bash
cd build
./client/test_client localhost 8080 --rate 100
```

---

## Detailed Build Steps

### Step 1: Create Build Directory
```bash
mkdir -p build
cd build
```

**Why separate build directory?**
- Keeps source tree clean
- Allows multiple build configurations (Debug/Release)
- Easy to clean (just delete build/)

### Step 2: Run CMake
```bash
cmake ..
```

**CMake will:**
- Check for C++17 compiler
- Generate Makefiles
- Configure include paths
- Set up threading libraries

**Expected output:**
```
-- The CXX compiler identification is GNU 11.4.0
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Check for working CXX compiler: /usr/bin/c++ - skipped
-- Detecting CXX compile features
-- Detecting CXX compile features - done
-- Configuring done
-- Generating done
-- Build files have been written to: /path/to/tech19_test/build
```

### Step 3: Compile
```bash
make -j$(nproc)
```

**Flags:**
- `-j$(nproc)` - Parallel compilation using all CPU cores

**Expected output:**
```
[ 16%] Building CXX object CMakeFiles/drone_server.dir/src/main.cpp.o
[ 33%] Building CXX object CMakeFiles/drone_server.dir/src/crc16.cpp.o
[ 50%] Building CXX object CMakeFiles/drone_server.dir/src/serialization.cpp.o
[ 66%] Building CXX object CMakeFiles/drone_server.dir/src/parser.cpp.o
[ 83%] Building CXX object CMakeFiles/drone_server.dir/src/network_listener.cpp.o
[100%] Building CXX object CMakeFiles/drone_server.dir/src/business_logic.cpp.o
[100%] Linking CXX executable drone_server
[100%] Built target drone_server
[100%] Built target test_client
```

### Step 4: Verify Build Artifacts
```bash
ls -lh
```

**You should see:**
```
drone_server        # Main server executable
client/test_client  # Test client executable
tests/run_tests     # Unit tests (if built with -DBUILD_TESTS=ON)
```

---

## Build Configurations

### Debug Build (with debugging symbols)
```bash
mkdir build-debug
cd build-debug
cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j$(nproc)
```

**Flags enabled:**
- `-g` - Debugging symbols
- `-O0` - No optimization

**Use for:**
- GDB debugging
- Valgrind analysis
- Development

### Release Build (optimized)
```bash
mkdir build-release
cd build-release
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
```

**Flags enabled:**
- `-O3` - Maximum optimization
- No debugging symbols

**Use for:**
- Performance testing
- Production deployment
- Benchmarking

---

## Building with Unit Tests

```bash
mkdir build
cd build
cmake -DBUILD_TESTS=ON ..
make -j$(nproc)

# Run tests
ctest --verbose

# Or run directly
./tests/run_tests
```

**Expected test output:**
```
=== Running Unit Tests ===

[PASS] crc16_known_values
[PASS] crc16_consistency
[PASS] endianness_conversion
[PASS] double_conversion
[PASS] serialize_deserialize_telemetry
[PASS] create_packet_structure
[PASS] invalid_telemetry_too_long_id

=== Test Summary ===
Passed: 7
Failed: 0
```

---

## Compiler Flags Explained

### Warnings
```cmake
-Wall       # Enable all common warnings
-Wextra     # Enable extra warnings
```

### Threading
```cmake
-pthread    # Enable POSIX threads
```

### Optimization Levels
- `-O0` - No optimization (Debug)
- `-O1` - Basic optimization
- `-O2` - Moderate optimization
- `-O3` - Aggressive optimization (Release)

---

## Troubleshooting

### Error: "CMake 3.15 or higher is required"
```bash
# Install newer CMake from Kitware repository
sudo apt remove --purge cmake
sudo apt install -y software-properties-common
sudo apt-add-repository 'deb https://apt.kitware.com/ubuntu/ focal main'
sudo apt update
sudo apt install -y cmake
```

### Error: "C++ compiler does not support C++17"
```bash
# Install GCC 9+
sudo apt install -y gcc-9 g++-9
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-9 90
sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-9 90
```

### Error: "pthread library not found"
```bash
# Install development libraries
sudo apt install -y libc6-dev
```

### Compilation Warnings
```bash
# Clean build
cd build
make clean
cmake ..
make -j$(nproc) 2>&1 | tee build.log
```

### Linker Errors
```bash
# Verify all source files are present
ls ../src/*.cpp
ls ../include/*.h

# Rebuild from scratch
rm -rf build
mkdir build && cd build
cmake .. && make
```

---

## Clean Build

```bash
# From project root
rm -rf build
mkdir build && cd build
cmake .. && make -j$(nproc)
```

---

## Cross-Compilation (Advanced)

### For Embedded Linux (ARM)
```bash
# Install cross-compiler
sudo apt install -y gcc-arm-linux-gnueabihf g++-arm-linux-gnueabihf

# Create toolchain file
cat > toolchain-arm.cmake << 'EOF'
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_C_COMPILER arm-linux-gnueabihf-gcc)
set(CMAKE_CXX_COMPILER arm-linux-gnueabihf-g++)
EOF

# Build
mkdir build-arm
cd build-arm
cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-arm.cmake ..
make -j$(nproc)
```

---

## Static Analysis

### Clang-Tidy
```bash
# Install
sudo apt install -y clang-tidy

# Run
cd build
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
clang-tidy ../src/*.cpp -- -I../include
```

### Cppcheck
```bash
# Install
sudo apt install -y cppcheck

# Run
cppcheck --enable=all --std=c++17 -I include src/
```

---

## Memory Leak Detection

### Valgrind
```bash
# Build with debug symbols
mkdir build-debug
cd build-debug
cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j$(nproc)

# Run server with valgrind
valgrind --leak-check=full --show-leak-kinds=all ./drone_server 8080

# In another terminal, run test client
./client/test_client localhost 8080 --rate 100 --duration 5

# Stop server with Ctrl+C and check valgrind output
```

**Expected clean output:**
```
HEAP SUMMARY:
    in use at exit: 0 bytes in 0 blocks
  total heap usage: 1,234 allocs, 1,234 frees, 123,456 bytes allocated

All heap blocks were freed -- no leaks are possible
```

---

## Thread Sanitizer

```bash
mkdir build-tsan
cd build-tsan
cmake -DCMAKE_CXX_FLAGS="-fsanitize=thread -g" ..
make -j$(nproc)

./drone_server 8080
# Run test client, verify no warnings
```

---

## Installation (Optional)

```bash
# Build
cd build
cmake -DCMAKE_INSTALL_PREFIX=/usr/local ..
make -j$(nproc)

# Install (requires sudo)
sudo make install

# Binaries installed to:
# /usr/local/bin/drone_server
# /usr/local/bin/test_client
```

---

## IDE Setup

### CLion
1. Open project root directory
2. CLion auto-detects CMakeLists.txt
3. Build → Build Project (Ctrl+F9)
4. Run → Edit Configurations → Add application → Select drone_server
5. Program arguments: `8080`

### VS Code
```bash
# Install extensions
code --install-extension ms-vscode.cpptools
code --install-extension ms-vscode.cmake-tools

# Open project
code .

# Use CMake sidebar to build
```

---

## Continuous Integration (CI)

### GitHub Actions Example
```yaml
name: Build

on: [push, pull_request]

jobs:
  build:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      - name: Install dependencies
        run: sudo apt update && sudo apt install -y build-essential cmake
      - name: Build
        run: |
          mkdir build && cd build
          cmake .. && make -j$(nproc)
      - name: Run tests
        run: cd build && ctest --verbose
```

---

## Build Time

**Expected build times** (Release mode, 4-core CPU):
- Clean build: ~5-10 seconds
- Incremental build: ~1-2 seconds
- With tests: ~10-15 seconds

---

## Binary Size

**Executable sizes** (Release mode, stripped):
- `drone_server`: ~150-250 KB
- `test_client`: ~100-150 KB

---

## Summary

**Recommended workflow:**
```bash
# Initial setup
git clone https://github.com/usuued/tech19_test.git
cd tech19_test

# Development build
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON ..
make -j$(nproc)

# Test
ctest --verbose

# Run
./drone_server 8080

# Production build
cd ..
mkdir build-release && cd build-release
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
```
