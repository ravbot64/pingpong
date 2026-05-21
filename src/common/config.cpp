#include "config.h"

#include <iostream>
#include <string>
#include <cstdlib>
#include <cerrno>
#include <climits>
#include <unistd.h>

namespace {

// Parse a strictly numeric integer (no leading whitespace tolerated post-getopt,
// no trailing garbage, must fit in int, must be in [min,max]).
bool parse_int_arg(const char* opt_name, const char* in, int min_v, int max_v, int& out) {
    if (in == nullptr || *in == '\0') {
        std::cerr << "Error: option -" << opt_name << " requires a numeric value" << std::endl;
        return false;
    }
    errno = 0;
    char* end = nullptr;
    long v = std::strtol(in, &end, 10);
    if (errno == ERANGE || end == in || end == nullptr || *end != '\0') {
        std::cerr << "Error: option -" << opt_name << ": '" << in
                  << "' is not a valid integer" << std::endl;
        return false;
    }
    if (v < static_cast<long>(min_v) || v > static_cast<long>(max_v)) {
        std::cerr << "Error: option -" << opt_name << " must be between "
                  << min_v << " and " << max_v << " (got " << v << ")" << std::endl;
        return false;
    }
    out = static_cast<int>(v);
    return true;
}

bool parse_double_arg(const char* opt_name, const char* in,
                      double min_v, double max_v, double& out) {
    if (in == nullptr || *in == '\0') {
        std::cerr << "Error: option -" << opt_name << " requires a numeric value" << std::endl;
        return false;
    }
    errno = 0;
    char* end = nullptr;
    double v = std::strtod(in, &end);
    if (errno == ERANGE || end == in || end == nullptr || *end != '\0') {
        std::cerr << "Error: option -" << opt_name << ": '" << in
                  << "' is not a valid number" << std::endl;
        return false;
    }
    if (v < min_v || v > max_v) {
        std::cerr << "Error: option -" << opt_name << " must be between "
                  << min_v << " and " << max_v << " (got " << v << ")" << std::endl;
        return false;
    }
    out = v;
    return true;
}

void print_usage(const char* program_name) {
    std::cerr <<
        "Usage:\n"
        "  Server: " << program_name << " -s [-B bind_addr] [-p port] [-i interval] [-l buffer]\n"
        "  Client: " << program_name << " -c <host> [-p port] [-t duration] [-i interval] [-l buffer]\n"
        "\n"
        "Options:\n"
        "  -s                Run as server\n"
        "  -c <host>         Run as client, connect to host (hostname or IPv4/IPv6 literal)\n"
        "  -B <bind_addr>    Server bind address (default: all interfaces)\n"
        "  -p <port>         Port to listen on / connect to (default: 5201, 1..65535)\n"
        "  -t <seconds>      Test duration in seconds (default: 10, " << kMinDuration << ".." << kMaxDuration << ")\n"
        "  -l <bytes>        Buffer size in bytes (default: 131072, " << kMinBufferSize << ".." << kMaxBufferSize << ")\n"
        "  -i <seconds>      Reporting interval (default: 1, " << kMinInterval << ".." << kMaxInterval << ")\n"
        "  -T <seconds>      Per-socket I/O timeout, 0 disables (default: 120)\n"
        "  -h                Show this help\n"
        "\n"
        "Examples:\n"
        "  " << program_name << " -s\n"
        "  " << program_name << " -s -B 127.0.0.1 -p 9000\n"
        "  " << program_name << " -c 192.168.1.5\n"
        "  " << program_name << " -c server.local -t 30 -i 2\n";
}

[[noreturn]] void fail(const char* program_name) {
    print_usage(program_name);
    std::exit(1);
}

} // namespace

Config parse_args(int argc, char* argv[]) {
    Config config;
    bool has_mode = false;
    int opt;

    // Keep getopt quiet — we print our own errors.
    opterr = 0;

    while ((opt = getopt(argc, argv, "sc:B:p:t:l:i:T:h")) != -1) {
        switch (opt) {
            case 's':
                if (has_mode) {
                    std::cerr << "Error: -s and -c are mutually exclusive" << std::endl;
                    fail(argv[0]);
                }
                config.mode = Mode::SERVER;
                has_mode = true;
                break;
            case 'c':
                if (has_mode) {
                    std::cerr << "Error: -s and -c are mutually exclusive" << std::endl;
                    fail(argv[0]);
                }
                config.mode = Mode::CLIENT;
                config.host = optarg ? optarg : "";
                has_mode = true;
                break;
            case 'B':
                config.bind_addr = optarg ? optarg : "";
                break;
            case 'p':
                if (!parse_int_arg("p", optarg, 1, 65535, config.port)) fail(argv[0]);
                break;
            case 't':
                if (!parse_int_arg("t", optarg, kMinDuration, kMaxDuration, config.duration)) fail(argv[0]);
                break;
            case 'l':
                if (!parse_int_arg("l", optarg, kMinBufferSize, kMaxBufferSize, config.buffer_size)) fail(argv[0]);
                break;
            case 'i':
                if (!parse_double_arg("i", optarg, kMinInterval, kMaxInterval, config.interval)) fail(argv[0]);
                break;
            case 'T':
                if (!parse_double_arg("T", optarg, 0.0, 3600.0, config.io_timeout_sec)) fail(argv[0]);
                break;
            case 'h':
                print_usage(argv[0]);
                std::exit(0);
            default: {
                if (optopt != 0) {
                    std::cerr << "Error: unknown or malformed option" << std::endl;
                }
                fail(argv[0]);
            }
        }
    }

    if (optind < argc) {
        std::cerr << "Error: unexpected positional argument '" << argv[optind] << "'" << std::endl;
        fail(argv[0]);
    }

    if (!has_mode) {
        std::cerr << "Error: must specify -s (server) or -c <host> (client)" << std::endl;
        fail(argv[0]);
    }

    if (config.mode == Mode::CLIENT && config.host.empty()) {
        std::cerr << "Error: client mode requires a host (-c <host>)" << std::endl;
        fail(argv[0]);
    }

    if (config.mode == Mode::CLIENT && !config.bind_addr.empty()) {
        std::cerr << "Error: -B is only valid in server mode" << std::endl;
        fail(argv[0]);
    }

    if (config.port < 1024) {
        std::cerr << "Warning: using privileged port <1024 may require root privileges." << std::endl;
    }

    return config;
}
