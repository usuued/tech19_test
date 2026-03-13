#ifndef PARSER_H
#define PARSER_H

#include "protocol.h"
#include "thread_safe_queue.h"
#include <vector>
#include <cstdint>
#include <atomic>

class StreamParser {
public:
    enum class State {
        SEARCHING_HEADER,
        READING_LENGTH,
        READING_PAYLOAD,
        READING_CRC
    };

    StreamParser(ThreadSafeQueue<std::vector<uint8_t>>& raw_queue,
                 ThreadSafeQueue<protocol::Telemetry>& parsed_queue,
                 std::atomic<bool>& shutdown_flag);

    // Main thread function
    void run();

    // Get statistics
    uint64_t get_packets_processed() const { return packets_processed_; }
    uint64_t get_crc_failures() const { return crc_failures_; }

private:
    // Process incoming data chunk
    void feed_data(const std::vector<uint8_t>& chunk);

    // State machine processing
    bool process_state();
    bool search_header();
    bool read_length();
    bool read_payload();
    bool read_crc();

    // Resynchronization (shift by one byte)
    void resynchronize();

    // Compact buffer (remove processed bytes)
    void compact_buffer();

    // State
    State state_;
    std::vector<uint8_t> buffer_;  // Accumulator buffer
    size_t parse_offset_;
    size_t packet_start_;  // Start of current packet being parsed
    uint16_t expected_payload_size_;

    // Statistics
    uint64_t packets_processed_;
    uint64_t crc_failures_;

    // Queues
    ThreadSafeQueue<std::vector<uint8_t>>& raw_queue_;
    ThreadSafeQueue<protocol::Telemetry>& parsed_queue_;
    std::atomic<bool>& shutdown_flag_;
};

#endif // PARSER_H
