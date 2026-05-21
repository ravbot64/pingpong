#include "tcp_server.h"

#include <iostream>
#include <vector>

#include "../common/signal.h"

TcpServer::TcpServer(const Config& config) : config_(config) {}

namespace {

bool open_listen_socket(Socket& s, const Config& cfg) {
    if (!s.create_and_bind(cfg.bind_addr, cfg.port)) return false;
    if (!s.listen(1)) return false;
    return true;
}

void print_listening_banner(const Config& cfg) {
    std::cout << "-------------------------------------------" << std::endl;
    std::cout << "Server listening on "
              << (cfg.bind_addr.empty() ? "*" : cfg.bind_addr)
              << ":" << cfg.port << std::endl;
    if (cfg.bind_addr.empty()) {
        std::cout << "(All interfaces. Use -B <addr> to restrict.)" << std::endl;
    }
    std::cout << "-------------------------------------------" << std::endl;
}

} // namespace

void TcpServer::run() {
    if (!open_listen_socket(listen_socket_, config_)) return;
    print_listening_banner(config_);

    while (!shutdown_requested()) {
        std::string client_ip;
        int client_port = 0;
        Socket client = listen_socket_.accept(client_ip, client_port);
        if (!client.is_valid()) {
            if (shutdown_requested()) break;
            continue;
        }

        // Stop accepting new connections while a test is running so a second
        // client doesn't silently queue and corrupt results.
        listen_socket_.close();

        std::cout << "Accepted connection from " << client_ip << ":" << client_port << std::endl;

        if (config_.io_timeout_sec > 0.0) {
            client.set_recv_timeout(config_.io_timeout_sec);
        }

        std::vector<char> buffer(config_.buffer_size);
        throughput_.set_interval(config_.interval);
        throughput_.start();

        Throughput::print_header();

        while (!shutdown_requested()) {
            ssize_t bytes_read = client.read(buffer.data(), buffer.size());
            if (bytes_read < 0) {
                std::cerr << "Error: recv failed during test." << std::endl;
                break;
            }
            if (bytes_read == 0) {
                // Peer half-closed: end of test.
                break;
            }

            // Anchor t = 0 to first byte so idle time before the client starts
            // sending doesn't get charged to the measurement window.
            throughput_.mark_first_byte();
            throughput_.add_bytes(static_cast<uint64_t>(bytes_read));

            std::string line;
            while (throughput_.check_interval(line)) {
                std::cout << line << std::endl;
            }
        }

        throughput_.stop();
        throughput_.report();
        std::cout << std::endl;
        std::cout << "Client disconnected." << std::endl;

        if (shutdown_requested()) break;

        if (!open_listen_socket(listen_socket_, config_)) return;
        print_listening_banner(config_);
    }

    if (shutdown_requested()) {
        std::cout << "Shutting down." << std::endl;
    }
}
