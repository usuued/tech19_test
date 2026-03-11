# Quick Start - Windows Build

## ⚠️ IMPORTANT NOTE

**The assignment requires Linux (Ubuntu).** This Windows build is for convenience only. For submission, you MUST test on Linux.

---

## Option 1: Use CLion (Easiest)

1. Open `tech19_test` folder in CLion
2. CLion will auto-detect CMakeLists.txt
3. Click **Build** → **Build Project** (or Ctrl+F9)
4. Run configurations will be automatically created
5. Run `drone_server` with argument `8080`
6. In another terminal: run `test_client` with args `localhost 8080 --rate 100`

---

## Option 2: Command Line (Git Bash)

### Method A: Use the build script

```bash
cd /c/Users/UriyaSuued/CLionProjects/test
chmod +x build_windows.sh
./build_windows.sh
```

### Method B: Manual build

```bash
cd /c/Users/UriyaSuued/CLionProjects/test

# Clean
rm -rf build
mkdir build && cd build

# Find your MinGW g++
which g++

# Configure (use the path from 'which g++')
cmake -G "Ninja" -DCMAKE_CXX_COMPILER="C:/path/to/your/g++.exe" ..

# Build
ninja

# Run
./drone_server.exe 8080
```

---

## Option 3: Use WSL2 (For Assignment Submission)

This is what you MUST use for the actual assignment:

```bash
# In Windows PowerShell or CMD
wsl

# In WSL terminal
cd /mnt/c/Users/UriyaSuued/CLionProjects/test

# Copy to WSL filesystem for better performance
cp -r . ~/tech19_test
cd ~/tech19_test

# Install tools (first time only)
sudo apt update
sudo apt install -y build-essential cmake

# Build
mkdir build && cd build
cmake ..
make -j$(nproc)

# Run
./drone_server 8080

# In another WSL terminal
cd ~/tech19_test/build
./client/test_client localhost 8080 --rate 100
```

---

## Troubleshooting

### "ninja: command not found"
Install ninja:
```bash
# Git Bash (Windows)
# Ninja should come with CLion. If not, download from:
# https://github.com/ninja-build/ninja/releases

# Or use Visual Studio generator instead:
cmake -G "Visual Studio 16 2019" ..
cmake --build .
```

### "CMake Error: CMAKE_CXX_COMPILER not set"
Your system has conflicting compilers. Use CLion or specify explicitly:
```bash
cmake -DCMAKE_CXX_COMPILER="C:/Program Files/mingw-w64/bin/g++.exe" ..
```

### "Operation not permitted" in WSL
You're working in `/mnt/c/` (Windows filesystem). Copy to WSL:
```bash
cp -r /mnt/c/Users/UriyaSuued/CLionProjects/test ~/tech19_test
cd ~/tech19_test
```

---

## What Was Changed for Windows

The code now includes `platform_compat.h` which:
- Uses **Winsock2** on Windows instead of POSIX sockets
- Provides Windows alternatives for Linux functions:
  - `closesocket()` instead of `close()`
  - `WSAStartup()` / `WSACleanup()` for initialization
  - `_byteswap_uint64()` instead of `htobe64()`
- Keeps original Linux code unchanged when building on Linux

---

## For Assignment Submission

**YOU MUST:**
1. Build and test in **WSL2 Ubuntu** (see Option 3 above)
2. Take screenshots of:
   - Server running
   - Client sending packets
   - Alerts being triggered
   - Performance test (1000 pps)
3. Verify with valgrind (no memory leaks)
4. Push final code to GitHub

The Windows build is just for development convenience. The assignment explicitly requires Linux.
