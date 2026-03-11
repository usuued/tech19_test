# Complete Development Plan - Counter-Drone TCP Stream Parser
## Tech19 C++ Embedded Homework Assignment (Version B)

**Repository:** `https://github.com/usuued/tech19_test`

---

## Project Overview

Multi-threaded C++ application for parsing continuous binary telemetry streams from drone units with automatic resynchronization on corruption.

### Technical Requirements Met

✅ C++17 standard
✅ Target Linux (Ubuntu)
✅ CMake build system
✅ Command-line execution
✅ Multi-threading (3 threads)
✅ Graceful shutdown handling
✅ RAII (no memory leaks)
✅ Test client with simulation
✅ Complete documentation

---

## File Structure

```
tech19_test/
├── CMakeLists.txt                      # Root build configuration
├── README.md                           # Main project README
├── .gitignore                          # Git ignore patterns
│
├── include/
│   ├── protocol.h                      # Packet format constants
│   ├── serialization.h                 # Binary encoding/decoding
│   ├── crc16.h                         # CRC-16-CCITT
│   ├── parser.h                        # State machine parser
│   ├── network_listener.h              # TCP server
│   ├── business_logic.h                # Drone tracking & alerts
│   └── thread_safe_queue.h             # Bounded queue template
│
├── src/
│   ├── serialization.cpp
│   ├── crc16.cpp
│   ├── parser.cpp
│   ├── network_listener.cpp
│   ├── business_logic.cpp
│   └── main.cpp                        # Entry point
│
├── client/
│   ├── CMakeLists.txt
│   └── test_client.cpp                 # Packet generator with corruption
│
├── tests/
│   ├── CMakeLists.txt
│   └── test_main.cpp                   # Unit tests
│
└── docs/
    ├── DEVELOPMENT_PLAN.md             # This file
    ├── ARCHITECTURE.md                 # System architecture
    ├── COMMITS.md                      # Git commit structure
    ├── BUILD.md                        # Build instructions
    └── PRODUCTION.md                   # Production improvements
```

---

## Development Phases

### Phase 1: Foundation (Completed)
- Project structure with CMake
- Protocol definitions
- CRC-16-CCITT implementation
- Serialization with network byte order

### Phase 2: Core Components (Completed)
- Thread-safe bounded queue
- State-machine parser with resynchronization
- Network listener (TCP server)
- Business logic (drone tracking + alerts)

### Phase 3: Integration (Completed)
- Main application with signal handling
- Thread orchestration
- Graceful shutdown sequence

### Phase 4: Testing Tools (Completed)
- Test client with configurable options
- Corruption injection
- Fragmentation simulation
- Unit tests for core components

### Phase 5: Documentation (Completed)
- Comprehensive README files
- Architecture diagrams
- Build instructions
- Production deployment guide

---

## Key Design Decisions

### 1. Network Byte Order (Big-Endian)
All numeric fields transmitted in network byte order for cross-platform compatibility.

### 2. Thread Confinement
Drone map is owned exclusively by business logic thread → no mutex overhead.

### 3. Bounded Queues with Backpressure
Producer blocks when queue is full → prevents memory exhaustion under load.

### 4. Shift-by-One Resynchronization
On CRC failure: advance buffer by 1 byte, search for next 0xAA55 header.

### 5. Single-Client TCP
Simplified design for assignment scope. Production would use epoll for multi-client.

---

## Testing Strategy

### Unit Tests
- CRC-16-CCITT correctness
- Endianness conversions
- Telemetry serialization/deserialization
- Packet structure validation

### Integration Tests
- Normal operation (no corruption)
- Corruption handling (5-10% error rate)
- Fragmentation across multiple recv() calls
- High throughput (1000 pps sustained)

### Performance Tests
- Measure packets/second
- Monitor queue depths
- Verify graceful degradation under overload

### Memory Tests
- Valgrind leak check
- Buffer growth monitoring
- Thread sanitizer for race conditions

---

## Performance Optimizations

| Optimization | Benefit |
|--------------|---------|
| Pre-allocated 8KB recv buffer | Reduce allocations |
| Bounded queues (2048 capacity) | Backpressure control |
| Move semantics in queues | Avoid deep copies |
| Thread confinement | No mutex on hot path |
| Buffer compaction at 4KB | Limit memory growth |

---

## Constraints & Trade-offs

### Constraints
1. Must handle fragmented packets
2. Must resynchronize on corruption
3. Must not crash on malformed input
4. Must handle 1000 packets/second
5. Must use C++17 (no external libraries for core)

### Trade-offs
| Decision | Advantage | Disadvantage |
|----------|-----------|--------------|
| Single client | Simpler code | Not production-ready |
| Blocking queues | Simple synchronization | Can cause backpressure |
| Network byte order | Cross-platform | Conversion overhead |
| Thread confinement | Fast access | Less flexible scaling |

---

## Timeline

**Total Development Time:** ~32 hours

- Protocol & serialization: 6h
- Parser implementation: 8h
- Threading & networking: 7h
- Business logic: 3h
- Test client: 4h
- Testing & debugging: 2h
- Documentation: 2h

---

## Next Steps for Production

1. **Multi-client support:** epoll-based event loop
2. **Persistent storage:** Write telemetry to TimescaleDB
3. **Metrics:** Prometheus exporter
4. **Security:** TLS encryption, authentication
5. **Configuration:** YAML config files
6. **Logging:** Structured JSON logs with spdlog
7. **Monitoring:** Grafana dashboards
8. **Testing:** Property-based tests, fuzz testing

---

## Verification Checklist

✅ Compiles with CMake
✅ Runs on Ubuntu 20.04+
✅ Handles 1000 pps sustained
✅ No memory leaks (valgrind clean)
✅ Graceful shutdown (Ctrl+C)
✅ Resynchronizes on corruption
✅ Alerts on altitude > 120m
✅ Alerts on speed > 50 m/s
✅ Test client works with all modes
✅ Unit tests pass
✅ Documentation complete
