#include "socket.h"

#include <iostream>
#include <cstring>       // memset
#include <unistd.h>      // ::close
#include <sys/socket.h>  // socket, bind, listen, accept, connect, send, recv
#include <netinet/in.h>  // sockaddr_in
#include <arpa/inet.h>   // inet_pton, inet_ntop

Socket::Socket() : fd_(-1) {}

Socket::Socket(int fd) : fd_(fd) {}

Socket::~Socket() {
    close();
}

// Move constructor
Socket::Socket(Socket&& other) noexcept : fd_(other.fd_) {
    other.fd_ = -1;
}

// Move assignment
Socket& Socket::operator=(Socket&& other) noexcept {
    if (this != &other) {
        close();
        fd_ = other.fd_;
        other.fd_ = -1;
    }
    return *this;
}

bool Socket::create() {
    fd_ = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd_ < 0) {
        std::cerr << "Error: socket() failed: " << strerror(errno) << std::endl;
        return false;
    }

    // Allow port reuse (avoids "address already in use" on restart)
    int opt = 1;
    if (::setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << "Warning: setsockopt(SO_REUSEADDR) failed: " << strerror(errno) << std::endl;
    }

    return true;
}

bool Socket::bind(int port) {
    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (::bind(fd_, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0) {
        std::cerr << "Error: bind() failed on port " << port << ": " << strerror(errno) << std::endl;
        return false;
    }

    return true;
}

bool Socket::listen(int backlog) {
    if (::listen(fd_, backlog) < 0) {
        std::cerr << "Error: listen() failed: " << strerror(errno) << std::endl;
        return false;
    }
    return true;
}

Socket Socket::accept(std::string& client_ip, int& client_port) {
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);

    int client_fd = ::accept(fd_, reinterpret_cast<struct sockaddr*>(&client_addr), &addr_len);
    if (client_fd < 0) {
        std::cerr << "Error: accept() failed: " << strerror(errno) << std::endl;
        return Socket(-1);
    }

    // Extract client's IP and port
    char ip_buf[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, ip_buf, sizeof(ip_buf));
    client_ip = ip_buf;
    client_port = ntohs(client_addr.sin_port);

    return Socket(client_fd);
}

bool Socket::connect(const std::string& host, int port) {
    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    if (inet_pton(AF_INET, host.c_str(), &addr.sin_addr) <= 0) {
        std::cerr << "Error: invalid address '" << host << "'" << std::endl;
        return false;
    }

    if (::connect(fd_, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0) {
        std::cerr << "Error: connect() failed to " << host << ":" << port << ": " << strerror(errno) << std::endl;
        return false;
    }

    return true;
}

ssize_t Socket::read(char* buffer, size_t length) {
    while (true) {
        ssize_t n = ::recv(fd_, buffer, length, 0);
        if (n < 0) {
            if (errno == EINTR) continue; // interrupted, retry
            std::cerr << "Error: recv() failed: " << strerror(errno) << std::endl;
        }
        return n;
    }
}

ssize_t Socket::write(const char* buffer, size_t length) {
    size_t total_sent = 0;

    // Handle partial writes — send() may not write everything in one call
    while (total_sent < length) {
        ssize_t sent = ::send(fd_, buffer + total_sent, length - total_sent, 0);
        if (sent < 0) {
            if (errno == EINTR) continue; // interrupted, retry
            std::cerr << "Error: send() failed: " << strerror(errno) << std::endl;
            return -1;
        }
        if (sent == 0) {
            // No progress; return bytes sent so far
            return static_cast<ssize_t>(total_sent);
        }
        total_sent += static_cast<size_t>(sent);
    }

    return static_cast<ssize_t>(total_sent);
}

bool Socket::shutdown_write() {
    if (fd_ < 0) return false;
    if (::shutdown(fd_, SHUT_WR) < 0) {
        std::cerr << "Warning: shutdown(SHUT_WR) failed: " << strerror(errno) << std::endl;
        return false;
    }
    return true;
}

void Socket::close() {
    if (fd_ >= 0) {
        ::close(fd_);
        fd_ = -1;
    }
}

int Socket::get_fd() const {
    return fd_;
}

bool Socket::is_valid() const {
    return fd_ >= 0;
}
