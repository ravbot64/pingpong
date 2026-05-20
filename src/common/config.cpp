#include "config.h"

#include <iostream>
#include <unistd.h>  // getopt
#include <cstdlib>   // atoi, exit

static void print_usage(const char* program_name) {
    std::cerr << "Usage:" << std::endl;
    std::cerr << "  Server: " << program_name << " -s [-p port]" << std::endl;
    std::cerr << "  Client: " << program_name << " -c <host> [-p port] [-t duration] [-l buffer_size] [-i interval]" << std::endl;
    std::cerr << std::endl;
    std::cerr << "Options:" << std::endl;
    std::cerr << "  -s            Run as server" << std::endl;
    std::cerr << "  -c <host>     Run as client, connect to <host>" << std::endl;
    std::cerr << "  -p <port>     Port to listen on / connect to (default: 5201)" << std::endl;
    std::cerr << "  -t <seconds>  Test duration in seconds (default: 10)" << std::endl;
    std::cerr << "  -l <bytes>    Buffer size in bytes (default: 131072)" << std::endl;
    std::cerr << "  -i <seconds>  Reporting interval in seconds (default: 1)" << std::endl;
    std::cerr << std::endl;
    std::cerr << "Examples:" << std::endl;
    std::cerr << "  " << program_name << " -s                        # start server on default port 5201" << std::endl;
    std::cerr << "  " << program_name << " -s -p 9000                # start server on port 9000" << std::endl;
    std::cerr << "  " << program_name << " -c 192.168.1.5            # connect to server, test for 10s" << std::endl;
    std::cerr << "  " << program_name << " -c 192.168.1.5 -t 30      # connect, test for 30 seconds" << std::endl;
    std::cerr << "  " << program_name << " -c 127.0.0.1 -p 9000      # connect to localhost on port 9000" << std::endl;
    std::cerr << "  " << program_name << " -c 10.0.0.1 -l 65536      # connect with 64KB buffer" << std::endl;
}

Config parse_args(int argc, char* argv[]) {
    Config config;
    bool has_mode = false;
    int opt;

    while ((opt = getopt(argc, argv, "sc:p:t:l:i:")) != -1) {
        switch (opt) {
            case 's':
                config.mode = Mode::SERVER;
                has_mode = true;
                break;
            case 'c':
                config.mode = Mode::CLIENT;
                config.host = optarg;
                has_mode = true;
                break;
            case 'p':
                config.port = std::atoi(optarg);
                break;
            case 't':
                config.duration = std::atoi(optarg);
                break;
            case 'l':
                config.buffer_size = std::atoi(optarg);
                break;
            case 'i':
                config.interval = std::atof(optarg);
                break;
            default:
                print_usage(argv[0]);
                exit(1);
        }
    }

    if (!has_mode) {
        std::cerr << "Error: must specify -s (server) or -c <host> (client)" << std::endl;
        print_usage(argv[0]);
        exit(1);
    }

    if (config.mode == Mode::CLIENT && config.host.empty()) {
        std::cerr << "Error: client mode requires a host (-c <host>)" << std::endl;
        print_usage(argv[0]);
        exit(1);
    }

    if (config.port < 1 || config.port > 65535) {
        std::cerr << "Error: port must be between 1 and 65535" << std::endl;
        exit(1);
    }
    if (config.port < 1024) {
        std::cerr << "Warning: using privileged port <1024 may require root privileges." << std::endl;
    }

    if (config.duration <= 0) {
        std::cerr << "Error: duration must be > 0" << std::endl;
        exit(1);
    }

    if (config.buffer_size <= 0) {
        std::cerr << "Error: buffer size must be > 0" << std::endl;
        exit(1);
    }

    if (config.interval <= 0.0) {
        std::cerr << "Error: interval must be > 0" << std::endl;
        exit(1);
    }

    return config;
}
