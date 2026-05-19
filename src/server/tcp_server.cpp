#include "tcp_server.h"

#include <iostream>
#include <vector>

TcpServer::TcpServer(const Config& config) : config_(config) {}

void TcpServer::run() {
    // 1. Create, bind, listen
    if (!listen_socket_.create()) return;
    if (!listen_socket_.bind(config_.port)) return;
    if (!listen_socket_.listen()) return;

    std::cout << "-------------------------------------------" << std::endl;
    std::cout << "Server listening on port " << config_.port << std::endl;
    std::cout << "-------------------------------------------" << std::endl;

    // 2. Accept a client (blocks until one connects)
    std::string client_ip;
    int client_port;
    Socket client = listen_socket_.accept(client_ip, client_port);
    if (!client.is_valid()) return;

    std::cout << "Accepted connection from " << client_ip << ":" << client_port << std::endl;

    // 3. Receive data as fast as possible
    std::vector<char> buffer(config_.buffer_size);
    throughput_.start();

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
    }

    // 4. Stop and report
    throughput_.stop();
    std::cout << std::endl;
    std::cout << "Client disconnected." << std::endl;
    throughput_.report();
}
