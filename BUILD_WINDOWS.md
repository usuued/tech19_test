# Building on Windows

**IMPORTANT:** This project is designed for **Linux only** (assignment requirement). Windows native builds are **not supported** because the code uses POSIX networking APIs.

## ✅ Recommended: Use WSL2 (Windows Subsystem for Linux)

### Prerequisites
1. Windows 10/11 with WSL2 installed
2. Ubuntu distribution in WSL

### Installation (if WSL not installed)
```powershell
# In PowerShell (Admin)
wsl --install
wsl --install -d Ubuntu
```

### Build Instructions in WSL2

```bash
# 1. Start WSL Ubuntu
wsl

# 2. Navigate to project (accessible via /mnt/c/)
cd /mnt/c/Users/UriyaSuued/CLionProjects/test

# 3. Install build dependencies (first time only)
sudo apt update
sudo apt install -y build-essential cmake git

# 4. Clean and build
rm -rf build
mkdir build && cd build
cmake ..
make -j$(nproc)

# 5. Run server
./drone_server 8080

# 6. In another WSL terminal: run test client
cd /mnt/c/Users/UriyaSuued/CLionProjects/test/build
./client/test_client localhost 8080 --rate 100
```

### Alternative: Use CLion with WSL Toolchain

1. Open CLion
2. Go to **File → Settings → Build, Execution, Deployment → Toolchains**
3. Click **+** → **WSL**
4. Select your WSL distribution
5. Click **OK**
6. Go to **File → Settings → Build, Execution, Deployment → CMake**
7. Select **WSL** as toolchain
8. Click **OK**
9. CLion will now build using WSL

---

## ❌ Windows Native Build NOT Supported

The following Linux-specific headers are used:
- `<arpa/inet.h>` - Network byte order conversion
- `<sys/socket.h>` - Socket APIs
- `<netinet/in.h>` - Internet addresses
- `<unistd.h>` - POSIX APIs
- `<endian.h>` - Endianness functions

These do **not exist** on Windows. Use WSL2.

---

## Testing

After building in WSL:

```bash
# Run unit tests
cd build
ctest --verbose

# Test with corruption
./client/test_client localhost 8080 --corrupt-rate 0.05

# Test with fragmentation
./client/test_client localhost 8080 --fragment

# Performance test (1000 pps)
./client/test_client localhost 8080 --rate 1000 --duration 60
```

---

## Troubleshooting

### "wsl: command not found"
WSL not installed. Run in PowerShell (Admin):
```powershell
wsl --install
```

### "build-essential not found"
Update apt first:
```bash
sudo apt update
sudo apt install -y build-essential
```

### "Permission denied" errors
Files created in Windows may have wrong permissions:
```bash
chmod +x /mnt/c/Users/UriyaSuued/CLionProjects/test/build/drone_server
```

---

## Why Linux Only?

This is an **embedded systems assignment** targeting Linux embedded platforms (like Raspberry Pi, industrial controllers, etc.). The assignment explicitly requires:

> Target Linux (Ubuntu preferred)

Windows native builds are out of scope for this assignment.
