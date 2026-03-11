#ifndef SERIALIZATION_H
#define SERIALIZATION_H

#include "protocol.h"
#include <vector>
#include <cstdint>

namespace serialization {

// Endianness conversion helpers
uint16_t host_to_net16(uint16_t value);
uint16_t net_to_host16(uint16_t value);
uint64_t host_to_net64(uint64_t value);
uint64_t net_to_host64(uint64_t value);

// Double conversion (uses uint64_t as intermediate)
uint64_t double_to_net(double value);
double net_to_double(uint64_t net_value);

// Serialize Telemetry to binary payload (without HEADER, LENGTH, CRC)
// Returns payload bytes only
std::vector<uint8_t> serialize_telemetry(const protocol::Telemetry& telemetry);

// Deserialize payload bytes to Telemetry
// Returns false if payload is malformed
bool deserialize_telemetry(const uint8_t* payload, size_t payload_size, protocol::Telemetry& telemetry);

// Create complete packet: [HEADER][LENGTH][PAYLOAD][CRC]
std::vector<uint8_t> create_packet(const protocol::Telemetry& telemetry);

} // namespace serialization

#endif // SERIALIZATION_H
