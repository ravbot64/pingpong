#include "tcp_server.h"

#include <iostream>
#include <vector>

TcpServer::TcpServer(const Config& config) : config_(config) {}

void TcpServer::run() {
    // 1. Create, bind, listen
    if (!listen_socket_.create()) return;
    if (!listen_socket_.bind(config_.port)) return;
    if (!listen_socket_.listen(1)) return;

    std::cout << "-------------------------------------------" << std::endl;
    std::cout << "Server listening on port " << config_.port << std::endl;
    std::cout << "-------------------------------------------" << std::endl;

    // Accept loop — handle one client at a time, then wait for the next
    while (true) {
        // 2. Accept a client (blocks until one connects)
        std::string client_ip;
        int client_port;
        Socket client = listen_socket_.accept(client_ip, client_port);
        if (!client.is_valid()) continue;

        // Stop accepting new connections while test is running.
        // Any new client gets "Connection refused" instead of silently
        // queuing in the kernel backlog (which causes corrupted results).
        listen_socket_.close();

        std::cout << "Accepted connection from " << client_ip << ":" << client_port << std::endl;

        // 3. Receive data as fast as possible
        std::vector<char> buffer(config_.buffer_size);
        throughput_.set_interval(config_.interval);
        throughput_.start();

        Throughput::print_header();

        while (true) {
            ssize_t bytes_read = client.read(buffer.data(), buffer.size());
            if (bytes_read < 0) {
                std::cerr << "Error: recv failed during test." << std::endl;
                break;
            }
            if (bytes_read == 0) {
                // Peer closed connection
                break;
            }
            throughput_.add_bytes(static_cast<size_t>(bytes_read));

            std::string line;
            if (throughput_.check_interval(line)) {
                std::cout << line << std::endl;
            }
        }

        // 4. Stop and report
        throughput_.stop();
        throughput_.report();
        std::cout << std::endl;
        std::cout << "Client disconnected." << std::endl;

        // 5. Re-open listen socket for next client
        if (!listen_socket_.create()) return;
        if (!listen_socket_.bind(config_.port)) return;
        if (!listen_socket_.listen(1)) return;

        std::cout << "-------------------------------------------" << std::endl;
        std::cout << "Server listening on port " << config_.port << std::endl;
        std::cout << "-------------------------------------------" << std::endl;
    }
}
