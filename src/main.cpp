#include "protocol.h"
#include "thread_safe_queue.h"
#include "network_listener.h"
#include "parser.h"
#include "business_logic.h"

#include <iostream>
#include <thread>
#include <atomic>
#include <csignal>
#include <cstdlib>

// Global shutdown flag
std::atomic<bool> g_shutdown_flag{false};

// Signal handler for graceful shutdown
void signal_handler(int signum) {
    std::cout << "\n[MAIN] Shutdown signal received (" << signum << ")\n";
    g_shutdown_flag = true;
}

int main(int argc, char* argv[]) {
    // Parse command line arguments
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <port>\n";
        std::cerr << "Example: " << argv[0] << " 8080\n";
        return 1;
    }

    uint16_t port = static_cast<uint16_t>(std::atoi(argv[1]));
    if (port == 0) {
        std::cerr << "[ERROR] Invalid port number\n";
        return 1;
    }

    // Register signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    std::cout << "=== Counter-Drone TCP Stream Parser ===\n";
    std::cout << "[MAIN] Starting server on port " << port << "\n";

    // Create thread-safe queues
    ThreadSafeQueue<std::vector<uint8_t>> raw_queue(2048);
    ThreadSafeQueue<protocol::Telemetry> parsed_queue(2048);

    // Create worker objects
    NetworkListener network(raw_queue, g_shutdown_flag);
    StreamParser parser(raw_queue, parsed_queue, g_shutdown_flag);
    BusinessLogic business(parsed_queue, g_shutdown_flag);

    // Launch threads
    std::thread network_thread([&network, port]() {
        network.run(port);
    });

    std::thread parser_thread([&parser]() {
        parser.run();
    });

    std::thread business_thread([&business]() {
        business.run();
    });

    // Wait for network thread to exit (either by shutdown signal or client disconnect)
    network_thread.join();
    std::cout << "[MAIN] Network thread exited\n";

    // Signal shutdown to queues
    raw_queue.shutdown();

    // Wait for parser thread
    parser_thread.join();
    std::cout << "[MAIN] Parser thread exited\n";

    // Signal shutdown to parsed queue
    parsed_queue.shutdown();

    // Wait for business logic thread
    business_thread.join();
    std::cout << "[MAIN] Business thread exited\n";

    std::cout << "[MAIN] Clean shutdown complete\n";
    return 0;
}
