# Counter-Drone TCP Stream Parser

Multi-threaded C++ application for parsing continuous binary telemetry streams with automatic resynchronization. Built for Tech19 C++ Embedded Developer assignment.

**Repository:** `https://github.com/usuued/tech19_test`

---

## Quick Start

```bash
# Clone and build
git clone https://github.com/usuued/tech19_test.git
cd tech19_test
mkdir build && cd build
cmake .. && make

# Run server
./drone_server 8080

# In another terminal: send test packets
./client/test_client 127.0.0.1 8080 --rate 1000
```

---

## Features

✅ **Streaming Parser** - Handles fragmented packets across multiple TCP reads
✅ **Resynchronization** - Automatically recovers from corrupted data
✅ **Multi-threaded** - Network, Parser, and Business Logic threads
✅ **High Performance** - Sustained 1000+ packets/second
✅ **Graceful Shutdown** - Clean SIGINT/SIGTERM handling
✅ **Memory Safe** - RAII, no leaks (valgrind clean)
✅ **Alert System** - Warnings for altitude > 120m, speed > 50 m/s

---

## Architecture

```
    ┌─────────────┐
TCP │  Network    │ recv() loop, blocking
───>│  Listener   │ Single client accepted
    └─────────────┘
           │
           │ push raw bytes
           ▼
    ┌─────────────┐
    │  Raw Queue  │ ThreadSafeQueue<vector<uint8_t>>
    │  (Bounded)  │ Capacity: 2048
    └─────────────┘
           │
           │ pop & parse
           ▼
    ┌─────────────┐
    │   Stream    │ State Machine:
    │   Parser    │ • SEARCHING_HEADER (0xAA55)
    │             │ • READING_LENGTH
    │   (CRC      │ • READING_PAYLOAD
    │ Validation) │ • READING_CRC
    │             │ • VALIDATING
    └─────────────┘
           │
           │ push valid packets
           ▼
    ┌─────────────┐
    │Parsed Queue │ ThreadSafeQueue<Telemetry>
    │             │ Capacity: 2048
    └─────────────┘
           │
           │ pop & process
           ▼
    ┌─────────────┐
    │  Business   │ • Update drone map
    │   Logic     │ • Check alerts
    │             │ • Log statistics
    └─────────────┘
            │
            ▼
     Logs & Alerts
```

### Packet Format (Network Byte Order)

```
┌──────────────────────────────────┐
│ HEADER: 0xAA55        2 bytes    │
├──────────────────────────────────┤
│ LENGTH: uint16_t      2 bytes    │
├──────────────────────────────────┤
│ PAYLOAD:                          │
│   drone_id_len        1 byte     │
│   drone_id            N bytes    │
│   latitude (double)   8 bytes    │
│   longitude (double)  8 bytes    │
│   altitude (double)   8 bytes    │
│   speed (double)      8 bytes    │
│   timestamp (uint64)  8 bytes    │
├──────────────────────────────────┤
│ CRC16 (CRC-16-CCITT)  2 bytes    │
└──────────────────────────────────┘
```

---

## Usage

### Server
```bash
./drone_server <port>

# Example
./drone_server 8080
```

**Output:**
```
=== Counter-Drone TCP Stream Parser ===
[MAIN] Starting server on port 8080
[NETWORK] Listening on port 8080
[NETWORK] Client connected from 127.0.0.1:54321
[STATS] Packets: 500 | Active drones: 12 | Rate: 100.0 pps
[ALERT] Drone UAV-037 altitude 145.2m (threshold 120.0m)
```

### Test Client
```bash
./client/test_client <host> <port> [options]

Options:
  --rate <pps>         Packets per second (default: 100)
  --duration <sec>     Test duration (default: 10)
  --corrupt-rate <r>   Corruption probability 0-1 (default: 0)
  --fragment           Enable packet fragmentation

Examples:
  ./client/test_client 127.0.0.1 8080
  ./client/test_client 127.0.0.1 8080 --rate 1000 --duration 60
  ./client/test_client 127.0.0.1 8080 --corrupt-rate 0.05 --fragment
```

---

## Parsing Logic & Resynchronization

### State Machine

```
  ┌──────────────────┐
  │ SEARCHING_HEADER │<──────────┐
  └──────────────────┘           │
           │                     │
           │ Found 0xAA55        │ CRC Fail
           ▼                     │ OR
  ┌──────────────────┐           │ Malformed
  │ READING_LENGTH   │           │
  └──────────────────┘           │
           │                     │
           ▼                     │
  ┌──────────────────┐           │
  │ READING_PAYLOAD  │           │
  └──────────────────┘           │
           │                     │
           ▼                     │
  ┌──────────────────┐           │
  │  READING_CRC     │           │
  └──────────────────┘           │
           │                     │
           ▼                     │
  ┌──────────────────┐           │
  │   VALIDATING     │───────────┘
  └──────────────────┘   (Shift buffer +1 byte)
           │
           │ CRC OK
           ▼
    Emit Telemetry
```

### Resynchronization Algorithm

**On CRC failure:**
1. Log error
2. Set `parse_offset = header_start + 1` **(shift by ONE byte)**
3. Return to SEARCHING_HEADER
4. Resume scanning for next 0xAA55

**Example:**
```
Buffer: [0x12 0xAA 0x55 ... <bad CRC> 0xAA 0x55 ...]

Action: Find 0xAA55 at offset 1 → parse → CRC fails
        Shift to offset 2 → search → find next 0xAA55
```

---

## Threading Model

| Thread | Responsibility | Synchronization |
|--------|----------------|-----------------|
| **NetworkListener** | recv() bytes from TCP | Pushes to raw_queue |
| **StreamParser** | State machine + CRC validation | Pops from raw_queue, pushes to parsed_queue |
| **BusinessLogic** | Drone tracking + alerts | Pops from parsed_queue, **thread-confined map** |

### Thread Safety
- **Queues:** Mutex + condition_variable (blocking)
- **Drone Map:** Thread-confined (no mutex needed)
- **Shutdown:** Atomic flag + sentinel values

### Graceful Shutdown
```
SIGINT → shutdown_flag = true
      → Network thread exits
      → raw_queue.shutdown()
      → Parser thread exits
      → parsed_queue.shutdown()
      → Business thread exits
      → main() joins all threads
```

---

## Performance

**Target:** 1000 packets/second

**Optimizations:**
- Bounded queues (backpressure)
- Pre-allocated buffers (8KB recv)
- Move semantics (avoid copies)
- Thread confinement (no mutex on hot path)
- Buffer compaction (prevent growth)

**Measurement:**
```bash
./client/test_client 127.0.0.1 8080 --rate 1000 --duration 60
# Server logs: "Rate: 1050.0 pps" ✓
```

---

## Testing

### Integration Tests
```bash
# Normal operation
./client/test_client 127.0.0.1 8080 --rate 100

# With corruption (5%)
./client/test_client 127.0.0.1 8080 --corrupt-rate 0.05

# Fragmentation
./client/test_client 127.0.0.1 8080 --fragment

# High throughput
./client/test_client 127.0.0.1 8080 --rate 1000 --duration 60
```

### Memory Leak Check
```bash
valgrind --leak-check=full ./drone_server 8080
# In another terminal: run test client
# Ctrl+C server, verify "All heap blocks were freed"
```

---

## Documentation

- **[ARCHITECTURE.md](docs/ARCHITECTURE.md)** - System architecture & design
- **[DEVELOPMENT_PLAN.md](docs/DEVELOPMENT_PLAN.md)** - Development process
- **[PRODUCTION.md](docs/PRODUCTION.md)** - Production improvements

---

## Requirements Met

✅ C++17
✅ Linux (Ubuntu)
✅ CMake build
✅ Command-line execution
✅ Multi-threading (3 threads)
✅ Graceful shutdown
✅ RAII (no memory leaks)
✅ Test client
✅ Documentation
✅ Streaming parser
✅ Resynchronization
✅ 1000 pps capable

---

## Project Structure

```
tech19_test/
├── CMakeLists.txt
├── README.md
├── include/
│   ├── protocol.h
│   ├── serialization.h
│   ├── crc16.h
│   ├── parser.h
│   ├── network_listener.h
│   ├── business_logic.h
│   └── thread_safe_queue.h
├── src/
│   ├── main.cpp
│   ├── crc16.cpp
│   ├── serialization.cpp
│   ├── parser.cpp
│   ├── network_listener.cpp
│   └── business_logic.cpp
├── client/
│   ├── CMakeLists.txt
│   └── test_client.cpp
├── tests/
│   ├── CMakeLists.txt
│   └── test_main.cpp
└── docs/
    ├── ARCHITECTURE.md
    ├── BUILD.md
    ├── COMMITS.md
    ├── DEVELOPMENT_PLAN.md
    └── PRODUCTION.md
```

---

## License

MIT License

---