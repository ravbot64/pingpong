#ifndef PINGPONG_SOCKET_H
#define PINGPONG_SOCKET_H

#include <string>

class Socket {
public:
    Socket();
    explicit Socket(int fd);  // Wrap an existing fd (used by accept)
    ~Socket();

    // Prevent copying (only one owner of an fd)
    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;

    // Allow moving
    Socket(Socket&& other) noexcept;
    Socket& operator=(Socket&& other) noexcept;

    // Core operations
    bool create();
    bool bind(int port);
    bool listen(int backlog = 5);
    Socket accept(std::string& client_ip, int& client_port);
    bool connect(const std::string& host, int port);

    // Data transfer
    ssize_t read(char* buffer, size_t length);
    ssize_t write(const char* buffer, size_t length);

    // Cleanup
    void close();

    // Partial shutdown (useful to signal EOF to peer)
    bool shutdown_write();

    // Accessors
    int get_fd() const;
    bool is_valid() const;

private:
    int fd_;
};

#endif
