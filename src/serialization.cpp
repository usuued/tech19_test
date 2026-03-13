#include "serialization.h"
#include "crc16.h"
#include "platform_compat.h"
#include <cstring>

namespace serialization {

uint16_t host_to_net16(uint16_t value) {
    return htons(value);
}

uint16_t net_to_host16(uint16_t value) {
    return ntohs(value);
}

uint64_t host_to_net64(uint64_t value) {
    return htobe64(value);
}

uint64_t net_to_host64(uint64_t value) {
    return be64toh(value);
}

uint64_t double_to_net(double value) {
    uint64_t bits;
    std::memcpy(&bits, &value, sizeof(double));
    return host_to_net64(bits);
}

double net_to_double(uint64_t net_value) {
    uint64_t host_bits = net_to_host64(net_value);
    double value;
    std::memcpy(&value, &host_bits, sizeof(double));
    return value;
}

// Helper to append 8-byte value to payload
static void append_uint64(std::vector<uint8_t>& payload, uint64_t value) {
    const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&value);
    payload.insert(payload.end(), bytes, bytes + 8);
}

std::vector<uint8_t> serialize_telemetry(const protocol::Telemetry& telemetry) {
    std::vector<uint8_t> payload;

    // Validate drone_id length
    if (telemetry.drone_id.length() > protocol::MAX_DRONE_ID_LENGTH) {
        return payload;  // Return empty on error
    }

    // drone_id_len (1 byte) + drone_id (N bytes)
    payload.push_back(static_cast<uint8_t>(telemetry.drone_id.length()));
    payload.insert(payload.end(), telemetry.drone_id.begin(), telemetry.drone_id.end());

    // Serialize doubles (latitude, longitude, altitude, speed)
    append_uint64(payload, double_to_net(telemetry.latitude));
    append_uint64(payload, double_to_net(telemetry.longitude));
    append_uint64(payload, double_to_net(telemetry.altitude));
    append_uint64(payload, double_to_net(telemetry.speed));

    // timestamp (8 bytes)
    append_uint64(payload, host_to_net64(telemetry.timestamp));

    return payload;
}

// Helper to read 8-byte value from payload
static uint64_t read_uint64(const uint8_t* data, size_t& offset) {
    uint64_t value;
    std::memcpy(&value, data + offset, 8);
    offset += 8;
    return value;
}

bool deserialize_telemetry(const uint8_t* payload, size_t payload_size, protocol::Telemetry& telemetry) {
    // Minimum size: 1 (id_len) + 0 (id) + 4*8 (doubles) + 8 (timestamp) = 41 bytes
    if (payload_size < 41) {
        return false;
    }

    size_t offset = 0;

    // drone_id_len
    uint8_t id_len = payload[offset++];

    if (id_len > protocol::MAX_DRONE_ID_LENGTH || offset + id_len + 40 > payload_size) {
        return false;
    }

    // drone_id
    telemetry.drone_id = std::string(reinterpret_cast<const char*>(payload + offset), id_len);
    offset += id_len;

    // Deserialize doubles (latitude, longitude, altitude, speed)
    telemetry.latitude = net_to_double(read_uint64(payload, offset));
    telemetry.longitude = net_to_double(read_uint64(payload, offset));
    telemetry.altitude = net_to_double(read_uint64(payload, offset));
    telemetry.speed = net_to_double(read_uint64(payload, offset));

    // timestamp
    telemetry.timestamp = net_to_host64(read_uint64(payload, offset));

    return true;
}

std::vector<uint8_t> create_packet(const protocol::Telemetry& telemetry) {
    std::vector<uint8_t> packet;

    // Serialize payload first
    std::vector<uint8_t> payload = serialize_telemetry(telemetry);
    if (payload.empty()) {
        return packet;  // Error
    }

    // HEADER (2 bytes, network order)
    uint16_t header_net = host_to_net16(protocol::HEADER_MAGIC);
    const uint8_t* header_bytes = reinterpret_cast<const uint8_t*>(&header_net);
    packet.insert(packet.end(), header_bytes, header_bytes + 2);

    // LENGTH (2 bytes, network order)
    uint16_t length = static_cast<uint16_t>(payload.size());
    uint16_t length_net = host_to_net16(length);
    const uint8_t* length_bytes = reinterpret_cast<const uint8_t*>(&length_net);
    packet.insert(packet.end(), length_bytes, length_bytes + 2);

    // PAYLOAD
    packet.insert(packet.end(), payload.begin(), payload.end());

    // CRC (computed over HEADER + LENGTH + PAYLOAD)
    uint16_t crc = crc::crc16_ccitt(packet.data(), packet.size());
    uint16_t crc_net = host_to_net16(crc);
    const uint8_t* crc_bytes = reinterpret_cast<const uint8_t*>(&crc_net);
    packet.insert(packet.end(), crc_bytes, crc_bytes + 2);

    return packet;
}

} // namespace serialization
