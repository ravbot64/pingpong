#ifndef PINGPONG_THROUGHPUT_H
#define PINGPONG_THROUGHPUT_H

#include <cstdint>
#include "timer.h"

class Throughput {
public:
    // Start tracking
    void start();

    // Add bytes transferred (call this after every read/write)
    void add_bytes(size_t count);

    // Stop tracking
    void stop();

    // Getters
    double get_bits_per_second() const;
    uint64_t get_total_bytes() const;
    double get_elapsed_seconds() const;

    // Print formatted summary to stdout
    void report() const;

private:
    Timer timer_;
    uint64_t total_bytes_ = 0;
    double elapsed_ = 0.0;  // cached on stop()
};

#endif
