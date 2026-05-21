#include "tcp_client.h"

#include <iostream>
#include <vector>

#include "../common/signal.h"
#include "../metrics/timer.h"

TcpClient::TcpClient(const Config& config) : config_(config) {}

void TcpClient::run() {
    std::cout << "Connecting to " << config_.host << " port " << config_.port << std::endl;

    if (!socket_.resolve_and_connect(config_.host, config_.port)) return;

    std::cout << "Connected." << std::endl;

    if (config_.io_timeout_sec > 0.0) {
        socket_.set_send_timeout(config_.io_timeout_sec);
    }

    // Send buffer is zero-filled. Content doesn't matter for a throughput test.
    std::vector<char> buffer(config_.buffer_size, 0);

    Timer duration_timer;
    duration_timer.start(config_.duration);
    throughput_.set_interval(config_.interval);
    throughput_.start();

    Throughput::print_header();

    while (!duration_timer.is_expired() && !shutdown_requested()) {
        ssize_t bytes_written = socket_.write(buffer.data(), buffer.size());
        if (bytes_written <= 0) {
            std::cerr << "Connection lost during test." << std::endl;
            break;
        }
        throughput_.add_bytes(static_cast<uint64_t>(bytes_written));

        std::string line;
        while (throughput_.check_interval(line)) {
            std::cout << line << std::endl;
        }
    }

    // Signal EOF to peer so the server can stop its read loop, then close.
    socket_.shutdown_write();
    socket_.close();

    throughput_.stop();
    throughput_.report();
}
