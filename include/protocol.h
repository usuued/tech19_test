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
constexpr size_t DRONE_ID_LEN_SIZE = 1;
constexpr size_t DOUBLE_SIZE = 8;
constexpr size_t UINT64_SIZE = 8;

// Minimum packet size: HEADER + LENGTH + (ID_LEN + lat + lon + alt + spd + ts) + CRC
constexpr size_t MIN_PACKET_SIZE = HEADER_SIZE + LENGTH_SIZE +
                                   (DRONE_ID_LEN_SIZE + 0 + 5 * DOUBLE_SIZE + UINT64_SIZE) +
                                   CRC_SIZE;

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
