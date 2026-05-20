#include "tcp_client.h"

#include <iostream>
#include <vector>
#include <cstring>  // memset

TcpClient::TcpClient(const Config& config) : config_(config) {}

void TcpClient::run() {
    // 1. Create and connect
    if (!socket_.create()) return;

    std::cout << "Connecting to " << config_.host << " port " << config_.port << std::endl;

    if (!socket_.connect(config_.host, config_.port)) return;

    std::cout << "Connected." << std::endl;

    // 2. Prepare send buffer (fill once with zeros)
    std::vector<char> buffer(config_.buffer_size, 0);

    // 3. Blast data for the configured duration
    Timer duration_timer;
    duration_timer.start(config_.duration);
    throughput_.set_interval(config_.interval);
    throughput_.start();

    Throughput::print_header();

    while (!duration_timer.is_expired()) {
        ssize_t bytes_written = socket_.write(buffer.data(), buffer.size());
        if (bytes_written <= 0) {
            std::cerr << "Connection lost during test." << std::endl;
            break;
        }
        throughput_.add_bytes(bytes_written);

        std::string line;
        if (throughput_.check_interval(line)) {
            std::cout << line << std::endl;
        }
    }

    // 4. Close connection (signals server that test is done)
    // Half-close to signal EOF, then fully close
    socket_.shutdown_write();
    socket_.close();

    // 5. Report
    throughput_.stop();
    throughput_.report();
}
