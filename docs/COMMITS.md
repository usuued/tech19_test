# Git Commit Structure

**Repository:** `https://github.com/usuued/tech19_test`

---

## Commit Strategy

Each commit represents a single, complete feature or fix that compiles successfully. Commits follow a logical progression from foundationto full system.

---

## Planned Commit Sequence

### Foundation Commits

**1. Initial CMake project structure with .gitignore**
- Create CMakeLists.txt with C++17 configuration
- Add comprehensive .gitignore for C++/CMake/IDE files
- Set up include/ and src/ directories

**2. Define protocol constants and wire format specification**
- Add protocol.h with packet format constants
- Document HEADER (0xAA55), LENGTH, PAYLOAD, CRC structure
- Define Telemetry struct matching assignment requirements
- Specify MAX_DRONE_ID_LENGTH and alert thresholds

### Core Implementation Commits

**3. Add Telemetry struct with network byte order serialization**
- Implement serialization.h interface
- Add endianness conversion helpers (htons, htobe64, etc.)
- Implement double ↔ network byte order conversion
- Create serialize_telemetry() and deserialize_telemetry()

**4. Implement CRC-16-CCITT calculation and validation**
- Add crc16.h and crc16.cpp
- Implement polynomial 0x1021 with initial value 0xFFFF
- Compute CRC over HEADER + LENGTH + PAYLOAD

**5. Add thread-safe bounded queue template with blocking semantics**
- Implement ThreadSafeQueue<T> in thread_safe_queue.h
- Use mutex + condition_variable for synchronization
- Add backpressure (block on full queue)
- Implement shutdown signaling

### Parser & Network Commits

**6. Implement state-machine parser with accumulator buffer**
- Create StreamParser class with 5 states
- SEARCHING_HEADER → READING_LENGTH → READING_PAYLOAD → READING_CRC → VALIDATING
- Buffer accumulation for fragmented packets
- Resynchronization on CRC failure (shift by one byte)

**7. Add TCP server with single-client blocking accept**
- Implement NetworkListener class
- Create socket, bind, listen on specified port
- Accept one client connection
- recv() loop pushing raw bytes to queue
- SO_REUSEADDR for quick restart

### Business Logic Commits

**8. Implement business logic with thread-confined drone map**
- Create DroneTracker and BusinessLogic classes
- Use std::unordered_map<string, Telemetry> (no mutex needed)
- Update drone state on valid packet
- Print statistics every 5 seconds

**9. Add alert system for altitude and speed thresholds**
- Check altitude > 120m
- Check speed > 50 m/s
- Print alerts to stdout with drone ID

### Integration Commits

**10. Implement SIGINT/SIGTERM handler for graceful shutdown**
- Add signal handler setting atomic shutdown flag
- Implement 5-step shutdown sequence
- Join threads in reverse order
- Clean RAII-based resource cleanup

### Testing Commits

**11. Add test client with valid packet generation**
- Implement TestClient class
- Generate random telemetry data
- Create packets using serialization::create_packet()
- Send at configurable rate

**12. Add corruption and fragmentation simulation to test client**
- --corrupt-rate option to inject random byte flips
- --fragment option to split packets across multiple sends
- Statistics reporting (sent, corrupted, actual rate)

**13. Implement performance measurement with packets-per-second logging**
- Track total packets processed
- Calculate elapsed time
- Log packets/sec every 5 seconds
- Verify 1000 pps capability

### Quality Assurance Commits

**14. Add unit tests for CRC, parser states, and serialization**
- Test CRC-16-CCITT with known values
- Test endianness conversions (round-trip)
- Test Telemetry serialize/deserialize
- Test invalid inputs (drone_id too long)

**15. Write comprehensive README with architecture diagrams**
- Build instructions
- Usage examples
- Architecture overview with ASCII diagrams
- State machine visualization
- Thread synchronization explanation

**16. Final validation: 1000 pps test and memory leak check with valgrind**
- Run test_client at 1000 pps for 60 seconds
- Run valgrind --leak-check=full
- Verify no memory leaks
- Verify graceful shutdown
- Update documentation with test results

---

## Commit Message Format

### Structure
```
<type>: <short description>

<detailed explanation if needed>
```

### Types
- **feat:** New feature
- **fix:** Bug fix
- **refactor:** Code restructuring
- **docs:** Documentation
- **test:** Testing
- **chore:** Build/tooling

### Examples

```bash
feat: Add CRC-16-CCITT implementation

Implements polynomial 0x1021 with initial value 0xFFFF.
Computes CRC over HEADER + LENGTH + PAYLOAD for packet validation.
```

```bash
feat: Implement state-machine parser with resynchronization

5-state parser handles:
- Fragmented packets across multiple recv() calls
- Multiple packets in single buffer
- CRC failures with shift-by-one resync
```

```bash
docs: Add comprehensive README with architecture diagrams

Includes:
- Build instructions
- Threading model with ASCII diagrams
- Parsing logic explanation
- Production deployment improvements
```

---

## Branch Strategy

### Main Branch
- `main` - Production-ready code
- All commits are atomic and compile successfully
- Each commit passes unit tests

### Development Flow
For this assignment, linear history on `main` is acceptable. In production:
- Feature branches: `feature/parser-state-machine`
- Bug fixes: `fix/crc-validation`
- Merge via Pull Requests with review

---

## Tag Strategy

### Version Tags
- `v1.0-submission` - Assignment submission version
- `v1.1-performance` - After 1000 pps validation
- `v1.2-production` - With multi-client support (future)

### Creating Tags
```bash
git tag -a v1.0-submission -m "Tech19 assignment submission"
git push origin v1.0-submission
```

---

## Commit Verification

Before each commit:
1. ✅ Code compiles without warnings
2. ✅ Unit tests pass (if applicable)
3. ✅ No memory leaks (quick valgrind check)
4. ✅ Follows C++17 style guidelines
5. ✅ Documentation updated if needed

---

## Example Commit History

```
* a1b2c3d (HEAD -> main, tag: v1.0-submission) docs: Final validation results
* d4e5f6g test: Add unit tests for all core components
* g7h8i9j feat: Add corruption and fragmentation to test client
* j0k1l2m feat: Implement test client with rate control
* m3n4o5p feat: Add SIGINT handler for graceful shutdown
* p6q7r8s feat: Implement alert system for altitude and speed
* s9t0u1v feat: Add business logic with drone tracking
* v2w3x4y feat: Implement TCP server with single-client accept
* y5z6a7b feat: Add state-machine parser with resynchronization
* b8c9d0e feat: Implement thread-safe bounded queue
* e1f2g3h feat: Add CRC-16-CCITT implementation
* h4i5j6k feat: Implement Telemetry serialization with endianness
* k7l8m9n feat: Define protocol constants and wire format
* n0o1p2q chore: Initial CMake project structure
```

---

## Post-Submission Commits

### Performance Tuning
- `perf: Optimize parser buffer compaction`
- `perf: Use move semantics in queue operations`

### Bug Fixes
- `fix: Handle EINTR in recv() loop`
- `fix: Validate payload size before allocation`

### Enhancements
- `feat: Add multi-client support with epoll`
- `feat: Add TLS encryption`
- `feat: Add Prometheus metrics exporter`

---

## Commit Statistics (Target)

- **Total Commits:** 16-20
- **Files Changed per Commit:** 1-5
- **Lines Added per Commit:** 50-500
- **Commit Frequency:** 1-3 per development session

---

## Repository Hygiene

### What NOT to Commit
- ❌ Build artifacts (build/, *.o, *.exe)
- ❌ IDE files (.vscode/, .idea/)
- ❌ Temporary files (*~, *.swp)
- ❌ Large binary files
- ❌ Credentials or secrets

### What TO Commit
- ✅ Source code (.h, .cpp)
- ✅ Build configuration (CMakeLists.txt)
- ✅ Documentation (.md files)
- ✅ Tests
- ✅ .gitignore
