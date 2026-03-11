#ifndef NETWORK_LISTENER_H
#define NETWORK_LISTENER_H

#include "thread_safe_queue.h"
#include <vector>
#include <cstdint>
#include <atomic>

class NetworkListener {
public:
    NetworkListener(ThreadSafeQueue<std::vector<uint8_t>>& raw_queue,
                    std::atomic<bool>& shutdown_flag);

    ~NetworkListener();

    // Main thread function
    void run(uint16_t port);

private:
    ThreadSafeQueue<std::vector<uint8_t>>& raw_queue_;
    std::atomic<bool>& shutdown_flag_;
    int server_fd_;
    int client_fd_;
};

#endif // NETWORK_LISTENER_H
