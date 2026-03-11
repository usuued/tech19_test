#include "serialization.h"
#include "crc16.h"
#include <cstring>
#include <arpa/inet.h>  // for htons, ntohs
#include <endian.h>     // for htobe64, be64toh

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

std::vector<uint8_t> serialize_telemetry(const protocol::Telemetry& telemetry) {
    std::vector<uint8_t> payload;

    // Validate drone_id length
    if (telemetry.drone_id.length() > protocol::MAX_DRONE_ID_LENGTH) {
        return payload;  // Return empty on error
    }

    // drone_id_len (1 byte)
    uint8_t id_len = static_cast<uint8_t>(telemetry.drone_id.length());
    payload.push_back(id_len);

    // drone_id (N bytes)
    payload.insert(payload.end(), telemetry.drone_id.begin(), telemetry.drone_id.end());

    // latitude (8 bytes, network byte order)
    uint64_t lat_net = double_to_net(telemetry.latitude);
    const uint8_t* lat_bytes = reinterpret_cast<const uint8_t*>(&lat_net);
    payload.insert(payload.end(), lat_bytes, lat_bytes + 8);

    // longitude (8 bytes)
    uint64_t lon_net = double_to_net(telemetry.longitude);
    const uint8_t* lon_bytes = reinterpret_cast<const uint8_t*>(&lon_net);
    payload.insert(payload.end(), lon_bytes, lon_bytes + 8);

    // altitude (8 bytes)
    uint64_t alt_net = double_to_net(telemetry.altitude);
    const uint8_t* alt_bytes = reinterpret_cast<const uint8_t*>(&alt_net);
    payload.insert(payload.end(), alt_bytes, alt_bytes + 8);

    // speed (8 bytes)
    uint64_t spd_net = double_to_net(telemetry.speed);
    const uint8_t* spd_bytes = reinterpret_cast<const uint8_t*>(&spd_net);
    payload.insert(payload.end(), spd_bytes, spd_bytes + 8);

    // timestamp (8 bytes)
    uint64_t ts_net = host_to_net64(telemetry.timestamp);
    const uint8_t* ts_bytes = reinterpret_cast<const uint8_t*>(&ts_net);
    payload.insert(payload.end(), ts_bytes, ts_bytes + 8);

    return payload;
}

bool deserialize_telemetry(const uint8_t* payload, size_t payload_size, protocol::Telemetry& telemetry) {
    if (payload_size < protocol::DRONE_ID_LEN_SIZE + 5 * protocol::DOUBLE_SIZE + protocol::UINT64_SIZE) {
        return false;  // Too small
    }

    size_t offset = 0;

    // drone_id_len
    uint8_t id_len = payload[offset++];

    if (id_len > protocol::MAX_DRONE_ID_LENGTH) {
        return false;  // Invalid length
    }

    if (offset + id_len + 5 * protocol::DOUBLE_SIZE + protocol::UINT64_SIZE > payload_size) {
        return false;  // Not enough data
    }

    // drone_id
    telemetry.drone_id = std::string(reinterpret_cast<const char*>(payload + offset), id_len);
    offset += id_len;

    // latitude
    uint64_t lat_net;
    std::memcpy(&lat_net, payload + offset, 8);
    telemetry.latitude = net_to_double(lat_net);
    offset += 8;

    // longitude
    uint64_t lon_net;
    std::memcpy(&lon_net, payload + offset, 8);
    telemetry.longitude = net_to_double(lon_net);
    offset += 8;

    // altitude
    uint64_t alt_net;
    std::memcpy(&alt_net, payload + offset, 8);
    telemetry.altitude = net_to_double(alt_net);
    offset += 8;

    // speed
    uint64_t spd_net;
    std::memcpy(&spd_net, payload + offset, 8);
    telemetry.speed = net_to_double(spd_net);
    offset += 8;

    // timestamp
    uint64_t ts_net;
    std::memcpy(&ts_net, payload + offset, 8);
    telemetry.timestamp = net_to_host64(ts_net);

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
