#include "network_listener.h"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cerrno>

NetworkListener::NetworkListener(ThreadSafeQueue<std::vector<uint8_t>>& raw_queue,
                                 std::atomic<bool>& shutdown_flag)
    : raw_queue_(raw_queue),
      shutdown_flag_(shutdown_flag),
      server_fd_(-1),
      client_fd_(-1) {}

NetworkListener::~NetworkListener() {
    if (client_fd_ >= 0) {
        close(client_fd_);
    }
    if (server_fd_ >= 0) {
        close(server_fd_);
    }
}

void NetworkListener::run(uint16_t port) {
    // Create socket
    server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd_ < 0) {
        std::cerr << "[NETWORK ERROR] Failed to create socket: " << strerror(errno) << "\n";
        return;
    }

    // Set SO_REUSEADDR to avoid "Address already in use" errors
    int opt = 1;
    if (setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << "[NETWORK WARNING] Failed to set SO_REUSEADDR\n";
    }

    // Bind
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_fd_, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr)) < 0) {
        std::cerr << "[NETWORK ERROR] Failed to bind: " << strerror(errno) << "\n";
        return;
    }

    // Listen
    if (listen(server_fd_, 1) < 0) {
        std::cerr << "[NETWORK ERROR] Failed to listen: " << strerror(errno) << "\n";
        return;
    }

    std::cout << "[NETWORK] Listening on port " << port << "\n";

    // Accept one client (blocking)
    sockaddr_in client_addr{};
    socklen_t client_len = sizeof(client_addr);
    client_fd_ = accept(server_fd_, reinterpret_cast<sockaddr*>(&client_addr), &client_len);

    if (client_fd_ < 0) {
        std::cerr << "[NETWORK ERROR] Failed to accept: " << strerror(errno) << "\n";
        return;
    }

    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
    std::cout << "[NETWORK] Client connected from " << client_ip << ":" << ntohs(client_addr.sin_port) << "\n";

    // Receive loop
    uint8_t buffer[8192];
    while (!shutdown_flag_) {
        ssize_t n = recv(client_fd_, buffer, sizeof(buffer), 0);

        if (n > 0) {
            // Push received data to queue
            raw_queue_.push(std::vector<uint8_t>(buffer, buffer + n));
        } else if (n == 0) {
            std::cout << "[NETWORK] Client disconnected\n";
            break;
        } else {
            if (errno == EINTR) {
                // Interrupted by signal, check shutdown flag
                continue;
            }
            std::cerr << "[NETWORK ERROR] recv failed: " << strerror(errno) << "\n";
            break;
        }
    }

    std::cout << "[NETWORK] Exiting\n";
}
