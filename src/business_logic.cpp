#include "business_logic.h"
#include <iostream>
#include <iomanip>

void DroneTracker::update(protocol::Telemetry&& telemetry) {
    // Check alerts before updating
    check_alerts(telemetry);

    // Update drone state (keyed by drone_id)
    drones_[telemetry.drone_id] = std::move(telemetry);
}

void DroneTracker::check_alerts(const protocol::Telemetry& t) {
    if (t.altitude > protocol::ALTITUDE_THRESHOLD) {
        std::cout << "[ALERT] Drone " << t.drone_id
                  << " altitude " << std::fixed << std::setprecision(1)
                  << t.altitude << "m (threshold " << protocol::ALTITUDE_THRESHOLD << "m)\n";
        alerts_triggered_++;
    }

    if (t.speed > protocol::SPEED_THRESHOLD) {
        std::cout << "[ALERT] Drone " << t.drone_id
                  << " speed " << std::fixed << std::setprecision(1)
                  << t.speed << " m/s (threshold " << protocol::SPEED_THRESHOLD << " m/s)\n";
        alerts_triggered_++;
    }
}

BusinessLogic::BusinessLogic(ThreadSafeQueue<protocol::Telemetry>& parsed_queue,
                             std::atomic<bool>& shutdown_flag)
    : parsed_queue_(parsed_queue),
      shutdown_flag_(shutdown_flag),
      packets_processed_(0),
      start_time_(std::chrono::steady_clock::now()),
      last_stats_time_(std::chrono::steady_clock::now()) {}

void BusinessLogic::run() {
    protocol::Telemetry telemetry;

    while (parsed_queue_.pop(telemetry)) {
        tracker_.update(std::move(telemetry));
        packets_processed_++;

        // Print statistics every 5 seconds
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - last_stats_time_).count();

        if (elapsed >= 5) {
            print_statistics();
            last_stats_time_ = now;
        }
    }

    // Print final statistics
    std::cout << "\n[BUSINESS] Final Statistics:\n";
    print_statistics();
}

void BusinessLogic::print_statistics() {
    auto now = std::chrono::steady_clock::now();
    auto total_elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - start_time_).count();

    double pps = (total_elapsed > 0) ? static_cast<double>(packets_processed_) / total_elapsed : 0.0;

    std::cout << "[STATS] Packets: " << packets_processed_
              << " | Active drones: " << tracker_.size()
              << " | Rate: " << std::fixed << std::setprecision(1) << pps << " pps\n";
}
