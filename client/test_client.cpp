#include "protocol.h"
#include "serialization.h"
#include "platform_compat.h"
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <chrono>
#include <random>
#include <thread>

// Windows doesn't have usleep, provide alternative
#ifdef _WIN32
#include <windows.h>
inline void usleep(unsigned int microseconds) {
    Sleep(microseconds / 1000);
}
#else
#include <unistd.h>
#endif

class TestClient {
public:
    TestClient(const std::string& host, uint16_t port)
        : host_(host), port_(port), sock_(-1),
          rng_(std::random_device{}()) {}

    ~TestClient() {
        if (sock_ >= 0) {
            close(sock_);
        }
    }

    bool connect() {
        sock_ = socket(AF_INET, SOCK_STREAM, 0);
        if (sock_ < 0) {
            std::cerr << "[ERROR] Failed to create socket\n";
            return false;
        }

        sockaddr_in server_addr{};
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port_);

        if (inet_pton(AF_INET, host_.c_str(), &server_addr.sin_addr) <= 0) {
            std::cerr << "[ERROR] Invalid address: " << host_ << "\n";
            return false;
        }

        if (::connect(sock_, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr)) < 0) {
            std::cerr << "[ERROR] Connection failed\n";
            return false;
        }

        std::cout << "[CLIENT] Connected to " << host_ << ":" << port_ << "\n";
        return true;
    }

    void run_test(int rate, int duration, double corrupt_rate, bool fragment) {
        std::cout << "[CLIENT] Test parameters:\n";
        std::cout << "  Rate: " << rate << " pps\n";
        std::cout << "  Duration: " << duration << " seconds\n";
        std::cout << "  Corruption rate: " << (corrupt_rate * 100) << "%\n";
        std::cout << "  Fragmentation: " << (fragment ? "enabled" : "disabled") << "\n";

        int total_packets = rate * duration;
        int sent = 0;
        int corrupted = 0;

        auto start = std::chrono::steady_clock::now();

        for (int i = 0; i < total_packets; ++i) {
            // Generate random telemetry
            protocol::Telemetry t = generate_telemetry(i);

            // Create packet
            std::vector<uint8_t> packet = serialization::create_packet(t);

            // Inject corruption?
            if (random_double() < corrupt_rate) {
                inject_corruption(packet);
                corrupted++;
            }

            // Send packet
            if (fragment && packet.size() > 10) {
                send_fragmented(packet);
            } else {
                send(sock_, packet.data(), packet.size(), 0);
            }

            sent++;

            // Sleep to maintain rate
            auto delay = std::chrono::microseconds(1'000'000 / rate);
            std::this_thread::sleep_for(delay);
        }

        auto end = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        std::cout << "\n[CLIENT] Test complete:\n";
        std::cout << "  Sent: " << sent << " packets\n";
        std::cout << "  Corrupted: " << corrupted << " packets\n";
        std::cout << "  Time: " << elapsed << " ms\n";
        std::cout << "  Actual rate: " << (sent * 1000.0 / elapsed) << " pps\n";
    }

private:
    protocol::Telemetry generate_telemetry(int seq) {
        protocol::Telemetry t;
        t.drone_id = "UAV-" + std::to_string(seq % 100);  // 100 unique drones
        t.latitude = random_double(-90.0, 90.0);
        t.longitude = random_double(-180.0, 180.0);
        t.altitude = random_double(0.0, 200.0);  // Some above threshold
        t.speed = random_double(0.0, 80.0);       // Some above threshold
        t.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        return t;
    }

    void inject_corruption(std::vector<uint8_t>& packet) {
        if (packet.empty()) return;

        // Corrupt random byte
        size_t pos = random_int(0, packet.size() - 1);
        packet[pos] ^= 0xFF;
    }

    void send_fragmented(const std::vector<uint8_t>& packet) {
        // Split packet into 2-3 fragments
        size_t split1 = packet.size() / 3;
        size_t split2 = 2 * packet.size() / 3;

        send(sock_, packet.data(), split1, 0);
        usleep(100);  // Small delay

        send(sock_, packet.data() + split1, split2 - split1, 0);
        usleep(100);

        send(sock_, packet.data() + split2, packet.size() - split2, 0);
    }

    double random_double(double min = 0.0, double max = 1.0) {
        std::uniform_real_distribution<double> dist(min, max);
        return dist(rng_);
    }

    size_t random_int(size_t min, size_t max) {
        std::uniform_int_distribution<size_t> dist(min, max);
        return dist(rng_);
    }

    std::string host_;
    uint16_t port_;
    int sock_;
    std::mt19937 rng_;
};

void print_usage(const char* prog) {
    std::cout << "Usage: " << prog << " <host> <port> [options]\n";
    std::cout << "Options:\n";
    std::cout << "  --rate <pps>         Packets per second (default: 100)\n";
    std::cout << "  --duration <sec>     Test duration in seconds (default: 10)\n";
    std::cout << "  --corrupt-rate <r>   Corruption probability 0-1 (default: 0)\n";
    std::cout << "  --fragment           Enable packet fragmentation\n";
    std::cout << "\nExamples:\n";
    std::cout << "  " << prog << " localhost 8080\n";
    std::cout << "  " << prog << " localhost 8080 --rate 1000 --duration 60\n";
    std::cout << "  " << prog << " localhost 8080 --rate 500 --corrupt-rate 0.05\n";
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        print_usage(argv[0]);
        return 1;
    }

    std::string host = argv[1];
    uint16_t port = static_cast<uint16_t>(std::atoi(argv[2]));

    // Parse options
    int rate = 100;
    int duration = 10;
    double corrupt_rate = 0.0;
    bool fragment = false;

    for (int i = 3; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "--rate" && i + 1 < argc) {
            rate = std::atoi(argv[++i]);
        } else if (arg == "--duration" && i + 1 < argc) {
            duration = std::atoi(argv[++i]);
        } else if (arg == "--corrupt-rate" && i + 1 < argc) {
            corrupt_rate = std::atof(argv[++i]);
        } else if (arg == "--fragment") {
            fragment = true;
        } else {
            std::cerr << "[ERROR] Unknown option: " << arg << "\n";
            print_usage(argv[0]);
            return 1;
        }
    }

    std::cout << "=== Counter-Drone Test Client ===\n";

    TestClient client(host, port);
    if (!client.connect()) {
        return 1;
    }

    client.run_test(rate, duration, corrupt_rate, fragment);

    std::cout << "[CLIENT] Done\n";
    return 0;
}
