#include "parser.h"
#include "serialization.h"
#include "crc16.h"
#include <iostream>
#include <cstring>

StreamParser::StreamParser(ThreadSafeQueue<std::vector<uint8_t>>& raw_queue,
                           ThreadSafeQueue<protocol::Telemetry>& parsed_queue,
                           std::atomic<bool>& shutdown_flag)
    : state_(State::SEARCHING_HEADER),
      parse_offset_(0),
      packet_start_(0),
      expected_payload_size_(0),
      packets_processed_(0),
      crc_failures_(0),
      raw_queue_(raw_queue),
      parsed_queue_(parsed_queue),
      shutdown_flag_(shutdown_flag) {}

void StreamParser::run() {
    std::vector<uint8_t> chunk;

    while (raw_queue_.pop(chunk)) {
        feed_data(chunk);
    }

    std::cout << "[PARSER] Exiting. Processed: " << packets_processed_
              << ", CRC failures: " << crc_failures_ << "\n";
}

void StreamParser::feed_data(const std::vector<uint8_t>& chunk) {
    // Append new data to buffer
    buffer_.insert(buffer_.end(), chunk.begin(), chunk.end());

    // Process as much as possible
    while (parse_offset_ < buffer_.size() && !shutdown_flag_) {
        bool advanced = process_state();
        if (!advanced) {
            break;  // Need more data
        }
    }

    // Compact buffer if it's getting large
    if (parse_offset_ > 4096) {
        compact_buffer();
    }
}

bool StreamParser::process_state() {
    switch (state_) {
        case State::SEARCHING_HEADER:
            return search_header();
        case State::READING_LENGTH:
            return read_length();
        case State::READING_PAYLOAD:
            return read_payload();
        case State::READING_CRC:
            return read_crc();
    }
    return false;
}

bool StreamParser::search_header() {
    // Scan for 0xAA55
    while (parse_offset_ + 1 < buffer_.size()) {
        if (buffer_[parse_offset_] == 0xAA && buffer_[parse_offset_ + 1] == 0x55) {
            packet_start_ = parse_offset_;
            parse_offset_ += 2;
            state_ = State::READING_LENGTH;
            return true;
        }
        parse_offset_++;
    }
    return false;  // Need more data
}

bool StreamParser::read_length() {
    // Need 2 more bytes for LENGTH
    if (parse_offset_ + 2 > buffer_.size()) {
        return false;  // Need more data
    }

    // Read LENGTH (network byte order)
    uint16_t length_net;
    std::memcpy(&length_net, buffer_.data() + parse_offset_, 2);
    expected_payload_size_ = serialization::net_to_host16(length_net);
    parse_offset_ += 2;

    // Validate length
    if (expected_payload_size_ > protocol::MAX_PAYLOAD_SIZE) {
        std::cerr << "[PARSER ERROR] Invalid payload size: " << expected_payload_size_ << "\n";
        resynchronize();
        return true;  // Continue processing
    }

    state_ = State::READING_PAYLOAD;
    return true;
}

bool StreamParser::read_payload() {
    // Need expected_payload_size_ more bytes
    if (parse_offset_ + expected_payload_size_ > buffer_.size()) {
        return false;  // Need more data
    }

    parse_offset_ += expected_payload_size_;
    state_ = State::READING_CRC;
    return true;
}

bool StreamParser::read_crc() {
    // Need 2 more bytes for CRC
    if (parse_offset_ + 2 > buffer_.size()) {
        return false;  // Need more data
    }

    // Extract and validate CRC
    size_t crc_offset = parse_offset_;
    uint16_t received_crc_net;
    std::memcpy(&received_crc_net, buffer_.data() + crc_offset, 2);
    uint16_t received_crc = serialization::net_to_host16(received_crc_net);

    // Compute CRC over HEADER + LENGTH + PAYLOAD (everything before the CRC)
    size_t crc_data_size = crc_offset - packet_start_;
    uint16_t computed_crc = crc::crc16_ccitt(buffer_.data() + packet_start_, crc_data_size);

    parse_offset_ += 2;

    if (received_crc != computed_crc) {
        std::cerr << "[PARSER ERROR] CRC mismatch. Expected: " << computed_crc
                  << ", Received: " << received_crc << "\n";
        resynchronize();
        return true;
    }

    // CRC valid - deserialize payload
    size_t payload_offset = packet_start_ + protocol::HEADER_SIZE + protocol::LENGTH_SIZE;
    const uint8_t* payload_ptr = buffer_.data() + payload_offset;

    protocol::Telemetry telemetry;
    if (!serialization::deserialize_telemetry(payload_ptr, expected_payload_size_, telemetry)) {
        std::cerr << "[PARSER ERROR] Failed to deserialize telemetry\n";
        resynchronize();
        return true;
    }

    // Success! Push to parsed queue
    parsed_queue_.push(std::move(telemetry));
    packets_processed_++;

    // Reset to search for next packet
    state_ = State::SEARCHING_HEADER;
    return true;
}

void StreamParser::resynchronize() {
    // Shift forward by ONE byte from packet start
    parse_offset_ = packet_start_ + 1;
    state_ = State::SEARCHING_HEADER;
    crc_failures_++;

    std::cerr << "[PARSER] Resynchronizing at offset " << parse_offset_ << "\n";
}

void StreamParser::compact_buffer() {
    // Remove processed bytes
    buffer_.erase(buffer_.begin(), buffer_.begin() + parse_offset_);
    packet_start_ = (packet_start_ >= parse_offset_) ? (packet_start_ - parse_offset_) : 0;
    parse_offset_ = 0;
}
