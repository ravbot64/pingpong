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
    std::string host = "";
    int port = 5201;
    int duration = 10;        // seconds (client only)
    int buffer_size = 131072; // 128 KB default (same as iperf)
};

Config parse_args(int argc, char* argv[]);

#endif
