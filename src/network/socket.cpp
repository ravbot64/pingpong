#include "socket.h"

#include <iostream>
#include <cstring>
#include <string>
#include <cerrno>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

namespace {

// Format a sockaddr (v4 or v6) into a printable IP + port.
bool format_peer(const struct sockaddr* sa, socklen_t salen,
                 std::string& ip_out, int& port_out) {
    char host[NI_MAXHOST] = {0};
    char serv[NI_MAXSERV] = {0};
    int rc = ::getnameinfo(sa, salen, host, sizeof(host), serv, sizeof(serv),
                           NI_NUMERICHOST | NI_NUMERICSERV);
    if (rc != 0) {
        return false;
    }
    ip_out = host;
    port_out = std::atoi(serv);
    return true;
}

bool apply_timeout(int fd, int optname, double seconds) {
    if (fd < 0) return false;
    struct timeval tv{};
    if (seconds > 0.0) {
        tv.tv_sec = static_cast<time_t>(seconds);
        double frac = seconds - static_cast<double>(tv.tv_sec);
        tv.tv_usec = static_cast<suseconds_t>(frac * 1e6);
    }
    if (::setsockopt(fd, SOL_SOCKET, optname, &tv, sizeof(tv)) < 0) {
        std::cerr << "Warning: setsockopt(SO_*TIMEO) failed: "
                  << std::strerror(errno) << std::endl;
        return false;
    }
    return true;
}

} // namespace

Socket::Socket() : fd_(-1) {}
Socket::Socket(int fd) : fd_(fd) {}

Socket::~Socket() {
    close();
}

Socket::Socket(Socket&& other) noexcept : fd_(other.fd_) {
    other.fd_ = -1;
}

Socket& Socket::operator=(Socket&& other) noexcept {
    if (this != &other) {
        close();
        fd_ = other.fd_;
        other.fd_ = -1;
    }
    return *this;
}

bool Socket::create_and_bind(const std::string& bind_addr, int port) {
    if (fd_ >= 0) {
        close();
    }

    struct addrinfo hints{};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV;
    // bind_addr must be a literal address (or empty -> wildcard via AI_PASSIVE).
    // We require numeric to avoid surprise DNS lookups on the bind path.
    hints.ai_flags |= AI_NUMERICHOST;

    const char* node = bind_addr.empty() ? nullptr : bind_addr.c_str();
    std::string port_str = std::to_string(port);

    struct addrinfo* res = nullptr;
    int rc = ::getaddrinfo(node, port_str.c_str(), &hints, &res);
    if (rc != 0) {
        std::cerr << "Error: getaddrinfo(" << (node ? node : "*") << "): "
                  << gai_strerror(rc) << std::endl;
        return false;
    }

    int last_errno = 0;
    for (struct addrinfo* ai = res; ai != nullptr; ai = ai->ai_next) {
        int fd = ::socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
        if (fd < 0) {
            last_errno = errno;
            continue;
        }

        int opt = 1;
        if (::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
            std::cerr << "Warning: setsockopt(SO_REUSEADDR) failed: "
                      << std::strerror(errno) << std::endl;
        }
        // On dual-stack systems, an unspecified IPv6 bind ("::") accepts v4
        // mapped addresses as well — make sure that's enabled explicitly.
        if (ai->ai_family == AF_INET6) {
            int v6only = 0;
            ::setsockopt(fd, IPPROTO_IPV6, IPV6_V6ONLY, &v6only, sizeof(v6only));
        }

        if (::bind(fd, ai->ai_addr, ai->ai_addrlen) < 0) {
            last_errno = errno;
            ::close(fd);
            continue;
        }

        fd_ = fd;
        ::freeaddrinfo(res);
        return true;
    }

    ::freeaddrinfo(res);
    if (last_errno != 0) errno = last_errno;
    std::cerr << "Error: bind() failed on "
              << (node ? node : "*") << ":" << port
              << ": " << std::strerror(errno) << std::endl;
    return false;
}

bool Socket::listen(int backlog) {
    if (::listen(fd_, backlog) < 0) {
        std::cerr << "Error: listen() failed: " << std::strerror(errno) << std::endl;
        return false;
    }
    return true;
}

Socket Socket::accept(std::string& client_ip, int& client_port) {
    struct sockaddr_storage client_addr{};
    socklen_t addr_len = sizeof(client_addr);

    int client_fd = ::accept(fd_,
                             reinterpret_cast<struct sockaddr*>(&client_addr),
                             &addr_len);
    if (client_fd < 0) {
        if (errno != EINTR) {
            std::cerr << "Error: accept() failed: " << std::strerror(errno) << std::endl;
        }
        return Socket(-1);
    }

    client_ip = "?";
    client_port = 0;
    format_peer(reinterpret_cast<struct sockaddr*>(&client_addr),
                addr_len, client_ip, client_port);

    return Socket(client_fd);
}

bool Socket::resolve_and_connect(const std::string& host, int port) {
    if (fd_ >= 0) {
        close();
    }

    struct addrinfo hints{};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_NUMERICSERV;

    std::string port_str = std::to_string(port);

    struct addrinfo* res = nullptr;
    int rc = ::getaddrinfo(host.c_str(), port_str.c_str(), &hints, &res);
    if (rc != 0) {
        std::cerr << "Error: getaddrinfo(" << host << "): "
                  << gai_strerror(rc) << std::endl;
        return false;
    }

    int last_errno = 0;
    for (struct addrinfo* ai = res; ai != nullptr; ai = ai->ai_next) {
        int fd = ::socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
        if (fd < 0) {
            last_errno = errno;
            continue;
        }

        if (::connect(fd, ai->ai_addr, ai->ai_addrlen) < 0) {
            last_errno = errno;
            ::close(fd);
            continue;
        }

        fd_ = fd;
        ::freeaddrinfo(res);
        return true;
    }

    ::freeaddrinfo(res);
    if (last_errno != 0) errno = last_errno;
    std::cerr << "Error: connect() failed to " << host << ":" << port
              << ": " << std::strerror(errno) << std::endl;
    return false;
}

ssize_t Socket::read(char* buffer, size_t length) {
    while (true) {
        ssize_t n = ::recv(fd_, buffer, length, 0);
        if (n < 0) {
            if (errno == EINTR) continue;
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                std::cerr << "Error: recv() timed out" << std::endl;
                return -1;
            }
            std::cerr << "Error: recv() failed: " << std::strerror(errno) << std::endl;
            return -1;
        }
        return n;
    }
}

ssize_t Socket::write(const char* buffer, size_t length) {
    size_t total_sent = 0;

    while (total_sent < length) {
        ssize_t sent = ::send(fd_, buffer + total_sent, length - total_sent,
                              MSG_NOSIGNAL);
        if (sent < 0) {
            if (errno == EINTR) continue;
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                std::cerr << "Error: send() timed out" << std::endl;
                return -1;
            }
            std::cerr << "Error: send() failed: " << std::strerror(errno) << std::endl;
            return -1;
        }
        if (sent == 0) {
            return static_cast<ssize_t>(total_sent);
        }
        total_sent += static_cast<size_t>(sent);
    }

    return static_cast<ssize_t>(total_sent);
}

bool Socket::set_recv_timeout(double seconds) {
    return apply_timeout(fd_, SO_RCVTIMEO, seconds);
}

bool Socket::set_send_timeout(double seconds) {
    return apply_timeout(fd_, SO_SNDTIMEO, seconds);
}

bool Socket::shutdown_write() {
    if (fd_ < 0) return false;
    if (::shutdown(fd_, SHUT_WR) < 0) {
        // EBADF / ENOTCONN are routine after a remote close; downgrade noise.
        if (errno != ENOTCONN) {
            std::cerr << "Warning: shutdown(SHUT_WR) failed: "
                      << std::strerror(errno) << std::endl;
        }
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

int Socket::get_fd() const { return fd_; }
bool Socket::is_valid() const { return fd_ >= 0; }
