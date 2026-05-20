#ifndef PINGPONG_THROUGHPUT_H
#define PINGPONG_THROUGHPUT_H

#include <cstdint>
#include <string>
#include "timer.h"

class Throughput {
public:
    // Configure reporting interval (call before start())
    void set_interval(double seconds);

    // Start tracking
    void start();

    // Add bytes transferred (call this after every read/write)
    void add_bytes(size_t count);

    // Check if the current interval has elapsed; if so, populate `line` and return true
    bool check_interval(std::string& line);

    // Stop tracking
    void stop();

    // Getters
    double get_bits_per_second() const;
    uint64_t get_total_bytes() const;
    double get_elapsed_seconds() const;

    // Print header row to stdout
    static void print_header();

    // Print separator + final aggregate summary row to stdout
    void report() const;

private:
    // Overall tracking
    Timer timer_;
    uint64_t total_bytes_ = 0;
    double elapsed_ = 0.0;  // cached on stop()

    // Interval tracking
    double interval_sec_ = 1.0;
    uint64_t interval_bytes_ = 0;
    Timer interval_timer_;
    double interval_start_ = 0.0;  // start offset of current interval (e.g. 1.00, 2.00)

    // Format one table row: "[  0]  0.00-1.00 sec   112 MBytes    940 Mbits/sec"
    static std::string format_row(double start, double end, uint64_t bytes, double bps);
};

#endif
