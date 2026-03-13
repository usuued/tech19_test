#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <cstdint>
#include <string>

namespace protocol {

// Packet header magic bytes
constexpr uint16_t HEADER_MAGIC = 0xAA55;

// Maximum sizes
constexpr size_t MAX_DRONE_ID_LENGTH = 64;
constexpr size_t MAX_PAYLOAD_SIZE = 1024;

// Packet structure sizes (in bytes)
constexpr size_t HEADER_SIZE = 2;
constexpr size_t LENGTH_SIZE = 2;
constexpr size_t CRC_SIZE = 2;

// Telemetry structure (matches assignment)
struct Telemetry {
    std::string drone_id;
    double latitude;
    double longitude;
    double altitude;
    double speed;
    uint64_t timestamp;
};

// Alert thresholds
constexpr double ALTITUDE_THRESHOLD = 120.0;  // meters
constexpr double SPEED_THRESHOLD = 50.0;       // m/s

} // namespace protocol

#endif // PROTOCOL_H
