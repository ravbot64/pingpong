#ifndef PINGPONG_UNITS_H
#define PINGPONG_UNITS_H

#include <string>
#include <cstdint>

// Converts bits per second to human-readable string (e.g. "943 Mbits/sec")
// Uses SI units (powers of 1000)
std::string format_bandwidth(double bits_per_second);

// Converts byte count to human-readable string (e.g. "1.10 GBytes")
// Uses binary units (powers of 1024)
std::string format_bytes(uint64_t bytes);

#endif
