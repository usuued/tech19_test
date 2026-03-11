#ifndef BUSINESS_LOGIC_H
#define BUSINESS_LOGIC_H

#include "protocol.h"
#include "thread_safe_queue.h"
#include <unordered_map>
#include <string>
#include <atomic>
#include <chrono>

class DroneTracker {
public:
    void update(protocol::Telemetry&& telemetry);
    size_t size() const { return drones_.size(); }

private:
    void check_alerts(const protocol::Telemetry& t);

    // Thread-confined: only accessed by business logic thread
    std::unordered_map<std::string, protocol::Telemetry> drones_;
    uint64_t alerts_triggered_ = 0;
};

class BusinessLogic {
public:
    BusinessLogic(ThreadSafeQueue<protocol::Telemetry>& parsed_queue,
                  std::atomic<bool>& shutdown_flag);

    // Main thread function
    void run();

    uint64_t get_packets_processed() const { return packets_processed_; }

private:
    void print_statistics();

    DroneTracker tracker_;
    ThreadSafeQueue<protocol::Telemetry>& parsed_queue_;
    std::atomic<bool>& shutdown_flag_;

    uint64_t packets_processed_;
    std::chrono::steady_clock::time_point start_time_;
    std::chrono::steady_clock::time_point last_stats_time_;
};

#endif // BUSINESS_LOGIC_H
