#ifndef PINGPONG_SOCKET_H
#define PINGPONG_SOCKET_H

#include <string>
#include <sys/types.h>

class Socket {
public:
    Socket();
    explicit Socket(int fd);
    ~Socket();

    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;

    Socket(Socket&& other) noexcept;
    Socket& operator=(Socket&& other) noexcept;

    // Create a listening (server-side) socket and bind it.
    // bind_addr may be "0.0.0.0", "::", a specific IPv4/IPv6 literal, or "" (defaults to wildcard).
    // The address family is chosen based on bind_addr (IPv6 wildcard accepts v4 mapped).
    bool create_and_bind(const std::string& bind_addr, int port);

    // Begin listening on a previously bound socket.
    bool listen(int backlog = 1);

    // Accept a single client. Returns an invalid Socket on error.
    Socket accept(std::string& client_ip, int& client_port);

    // Resolve `host` (hostname, IPv4 literal, or IPv6 literal) and connect.
    // Tries each addrinfo result until one succeeds. Returns true on success.
    bool resolve_and_connect(const std::string& host, int port);

    // Data transfer.
    ssize_t read(char* buffer, size_t length);
    ssize_t write(const char* buffer, size_t length);

    // Apply a recv/send timeout to this socket (0 disables).
    bool set_recv_timeout(double seconds);
    bool set_send_timeout(double seconds);

    void close();
    bool shutdown_write();

    int get_fd() const;
    bool is_valid() const;

private:
    int fd_;
};

#endif
