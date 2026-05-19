#include "units.h"

#include <cstdio>  // snprintf

std::string format_bandwidth(double bits_per_second) {
    char buf[64];

    if (bits_per_second >= 1e9) {
        std::snprintf(buf, sizeof(buf), "%.2f Gbits/sec", bits_per_second / 1e9);
    } else if (bits_per_second >= 1e6) {
        std::snprintf(buf, sizeof(buf), "%.2f Mbits/sec", bits_per_second / 1e6);
    } else if (bits_per_second >= 1e3) {
        std::snprintf(buf, sizeof(buf), "%.2f Kbits/sec", bits_per_second / 1e3);
    } else {
        std::snprintf(buf, sizeof(buf), "%.2f bits/sec", bits_per_second);
    }

    return std::string(buf);
}

std::string format_bytes(uint64_t bytes) {
    char buf[64];

    if (bytes >= (1ULL << 30)) {
        std::snprintf(buf, sizeof(buf), "%.2f GBytes", static_cast<double>(bytes) / (1ULL << 30));
    } else if (bytes >= (1ULL << 20)) {
        std::snprintf(buf, sizeof(buf), "%.2f MBytes", static_cast<double>(bytes) / (1ULL << 20));
    } else if (bytes >= (1ULL << 10)) {
        std::snprintf(buf, sizeof(buf), "%.2f KBytes", static_cast<double>(bytes) / (1ULL << 10));
    } else {
        std::snprintf(buf, sizeof(buf), "%llu Bytes", static_cast<unsigned long long>(bytes));
    }

    return std::string(buf);
}
