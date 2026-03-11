#include "crc16.h"
#include "serialization.h"
#include "protocol.h"
#include <iostream>
#include <cassert>
#include <cstring>

// Simple test framework
int tests_passed = 0;
int tests_failed = 0;

#define TEST(name) \
    void test_##name(); \
    void run_test_##name() { \
        try { \
            test_##name(); \
            std::cout << "[PASS] " << #name << "\n"; \
            tests_passed++; \
        } catch (const std::exception& e) { \
            std::cout << "[FAIL] " << #name << ": " << e.what() << "\n"; \
            tests_failed++; \
        } \
    } \
    void test_##name()

#define ASSERT_EQ(a, b) \
    if ((a) != (b)) { \
        throw std::runtime_error(std::string("Expected ") + std::to_string(a) + " == " + std::to_string(b)); \
    }

#define ASSERT_TRUE(cond) \
    if (!(cond)) { \
        throw std::runtime_error("Condition failed"); \
    }

// CRC16 Tests
TEST(crc16_known_values) {
    const uint8_t data1[] = {0xAA, 0x55};
    uint16_t crc1 = crc::crc16_ccitt(data1, 2);
    // Just verify it computes something
    ASSERT_TRUE(crc1 != 0);

    // Test empty data
    uint16_t crc_empty = crc::crc16_ccitt(nullptr, 0);
    ASSERT_EQ(crc_empty, 0xFFFF);  // Should return initial value
}

TEST(crc16_consistency) {
    const uint8_t data[] = {0x01, 0x02, 0x03, 0x04};
    uint16_t crc1 = crc::crc16_ccitt(data, 4);
    uint16_t crc2 = crc::crc16_ccitt(data, 4);
    ASSERT_EQ(crc1, crc2);  // Same input should give same output
}

// Serialization Tests
TEST(endianness_conversion) {
    uint16_t val16 = 0x1234;
    uint16_t net16 = serialization::host_to_net16(val16);
    uint16_t back16 = serialization::net_to_host16(net16);
    ASSERT_EQ(val16, back16);

    uint64_t val64 = 0x123456789ABCDEF0ULL;
    uint64_t net64 = serialization::host_to_net64(val64);
    uint64_t back64 = serialization::net_to_host64(net64);
    ASSERT_EQ(val64, back64);
}

TEST(double_conversion) {
    double original = 123.456;
    uint64_t net = serialization::double_to_net(original);
    double converted = serialization::net_to_double(net);
    ASSERT_TRUE(std::abs(original - converted) < 0.0001);
}

TEST(serialize_deserialize_telemetry) {
    protocol::Telemetry original;
    original.drone_id = "UAV-001";
    original.latitude = 37.7749;
    original.longitude = -122.4194;
    original.altitude = 150.0;
    original.speed = 25.5;
    original.timestamp = 1234567890;

    // Serialize
    std::vector<uint8_t> payload = serialization::serialize_telemetry(original);
    ASSERT_TRUE(!payload.empty());

    // Deserialize
    protocol::Telemetry deserialized;
    bool success = serialization::deserialize_telemetry(payload.data(), payload.size(), deserialized);
    ASSERT_TRUE(success);

    // Verify
    ASSERT_EQ(original.drone_id, deserialized.drone_id);
    ASSERT_TRUE(std::abs(original.latitude - deserialized.latitude) < 0.0001);
    ASSERT_TRUE(std::abs(original.longitude - deserialized.longitude) < 0.0001);
    ASSERT_TRUE(std::abs(original.altitude - deserialized.altitude) < 0.0001);
    ASSERT_TRUE(std::abs(original.speed - deserialized.speed) < 0.0001);
    ASSERT_EQ(original.timestamp, deserialized.timestamp);
}

TEST(create_packet_structure) {
    protocol::Telemetry t;
    t.drone_id = "TEST";
    t.latitude = 0.0;
    t.longitude = 0.0;
    t.altitude = 0.0;
    t.speed = 0.0;
    t.timestamp = 0;

    std::vector<uint8_t> packet = serialization::create_packet(t);
    ASSERT_TRUE(packet.size() >= protocol::MIN_PACKET_SIZE);

    // Verify header
    ASSERT_EQ(packet[0], 0xAA);
    ASSERT_EQ(packet[1], 0x55);

    // Verify we can extract length
    uint16_t length_net;
    std::memcpy(&length_net, packet.data() + 2, 2);
    uint16_t length = serialization::net_to_host16(length_net);
    ASSERT_TRUE(length > 0);
}

TEST(invalid_telemetry_too_long_id) {
    protocol::Telemetry t;
    t.drone_id = std::string(100, 'X');  // Too long
    t.latitude = 0.0;
    t.longitude = 0.0;
    t.altitude = 0.0;
    t.speed = 0.0;
    t.timestamp = 0;

    std::vector<uint8_t> payload = serialization::serialize_telemetry(t);
    ASSERT_TRUE(payload.empty());  // Should fail
}

int main() {
    std::cout << "=== Running Unit Tests ===\n\n";

    run_test_crc16_known_values();
    run_test_crc16_consistency();
    run_test_endianness_conversion();
    run_test_double_conversion();
    run_test_serialize_deserialize_telemetry();
    run_test_create_packet_structure();
    run_test_invalid_telemetry_too_long_id();

    std::cout << "\n=== Test Summary ===\n";
    std::cout << "Passed: " << tests_passed << "\n";
    std::cout << "Failed: " << tests_failed << "\n";

    return (tests_failed == 0) ? 0 : 1;
}
