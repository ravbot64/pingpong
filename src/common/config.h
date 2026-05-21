#ifndef PINGPONG_CONFIG_H
#define PINGPONG_CONFIG_H

#include <string>
#include <cstdint>

enum class Mode {
    SERVER,
    CLIENT
};

struct Config {
    Mode mode = Mode::SERVER;

    // Client target host (hostname, IPv4, or IPv6 literal).
    std::string host = "";

    // Server bind address. Empty string means wildcard (all interfaces).
    // Default mirrors iperf3 behavior; pass -B to lock down.
    std::string bind_addr = "";

    int port = 5201;
    int duration = 10;           // seconds, client only
    int buffer_size = 131072;    // 128 KiB, same as iperf default
    double interval = 1.0;       // reporting interval, seconds

    // Per-socket I/O timeout (seconds). 0 disables. Defaults to a generous
    // value so a wedged peer can't pin the server forever.
    double io_timeout_sec = 120.0;
};

// Parses argv and exits the process on invalid input with a usage message.
Config parse_args(int argc, char* argv[]);

// Hard caps used by parse_args. Exposed for tests / docs.
constexpr int kMinBufferSize = 1;
constexpr int kMaxBufferSize = 16 * 1024 * 1024;  // 16 MiB
constexpr int kMinDuration = 1;
constexpr int kMaxDuration = 24 * 60 * 60;        // 1 day
constexpr double kMinInterval = 0.01;             // 10 ms
constexpr double kMaxInterval = 60.0;             // 60 s

#endif
