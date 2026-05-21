#ifndef PINGPONG_THROUGHPUT_H
#define PINGPONG_THROUGHPUT_H

#include <cstdint>
#include <string>
#include "timer.h"

// Throughput tracks bytes transferred over a TCP test and emits iperf-style
// interval rows on a fixed time grid anchored at t = 0.
//
// Lifecycle:
//   set_interval(seconds);
//   start();                      // anchors t = 0 (or call mark_first_byte() later)
//   loop:
//       add_bytes(n);
//       while (check_interval(line)) { print(line); }   // may emit multiple rows
//   stop();
//   report();                     // prints aggregate
class Throughput {
public:
    void set_interval(double seconds);

    // Start tracking now. t = 0 is anchored at this call.
    void start();

    // Re-anchor t = 0 to "now". Useful on the server side where start()
    // is called before the first byte arrives — calling this on the first
    // successful recv() keeps idle time out of the measurement.
    // Safe to call exactly once; subsequent calls are no-ops.
    void mark_first_byte();

    // Add bytes transferred. Counts toward both the overall and current-interval totals.
    void add_bytes(uint64_t count);

    // If at least one full reporting interval has completed since the last
    // call, populate `line` with the next bucket's row and return true.
    // Call in a loop after add_bytes() to drain all completed intervals.
    bool check_interval(std::string& line);

    // Stop tracking. Caches the final elapsed time.
    void stop();

    // Getters (valid after stop()).
    double get_bits_per_second() const;
    uint64_t get_total_bytes() const;
    double get_elapsed_seconds() const;

    static void print_header();

    // Prints any final partial-interval row + separator + aggregate summary.
    void report() const;

private:
    Timer timer_;
    double interval_sec_ = 1.0;

    bool started_ = false;
    bool first_byte_seen_ = false;

    uint64_t total_bytes_ = 0;
    double elapsed_ = 0.0;     // cached on stop()

    // Fixed-grid interval state. The Nth interval is [N*interval_sec, (N+1)*interval_sec].
    // next_boundary_sec_ is the right edge of the currently-accumulating bucket.
    uint64_t interval_bytes_ = 0;
    double interval_start_sec_ = 0.0;
    double next_boundary_sec_ = 0.0;

    static std::string format_row(double start, double end, uint64_t bytes, double bps);
};

#endif
