#include "throughput.h"
#include "../common/units.h"

#include <iostream>
#include <cstdio>

void Throughput::set_interval(double seconds) {
    if (seconds > 0.0) {
        interval_sec_ = seconds;
    }
}

void Throughput::start() {
    total_bytes_ = 0;
    elapsed_ = 0.0;
    interval_bytes_ = 0;
    interval_start_sec_ = 0.0;
    next_boundary_sec_ = interval_sec_;
    first_byte_seen_ = false;
    started_ = true;
    timer_.start();
}

void Throughput::mark_first_byte() {
    if (!started_ || first_byte_seen_) return;
    first_byte_seen_ = true;
    // Re-anchor t = 0 to "now" so idle time before the first byte doesn't
    // count toward elapsed_seconds().
    timer_.reset();
}

void Throughput::add_bytes(uint64_t count) {
    total_bytes_ += count;
    interval_bytes_ += count;
}

bool Throughput::check_interval(std::string& line) {
    if (!started_) return false;

    double now_sec = timer_.elapsed_seconds();
    if (now_sec < next_boundary_sec_) {
        return false;
    }

    // Emit the bucket that just closed: [interval_start_sec_, next_boundary_sec_].
    // Bandwidth is computed against the configured interval width, not the
    // measured wall-clock delta, so each bucket reports a clean rate.
    double bps = (interval_sec_ > 0.0)
        ? (static_cast<double>(interval_bytes_) * 8.0) / interval_sec_
        : 0.0;

    line = format_row(interval_start_sec_, next_boundary_sec_, interval_bytes_, bps);

    interval_bytes_ = 0;
    interval_start_sec_ = next_boundary_sec_;
    next_boundary_sec_ += interval_sec_;

    return true;
}

void Throughput::stop() {
    if (!started_) return;
    elapsed_ = timer_.elapsed_seconds();
    started_ = false;
}

double Throughput::get_bits_per_second() const {
    if (elapsed_ <= 0.0) return 0.0;
    return (static_cast<double>(total_bytes_) * 8.0) / elapsed_;
}

uint64_t Throughput::get_total_bytes() const {
    return total_bytes_;
}

double Throughput::get_elapsed_seconds() const {
    return elapsed_;
}

void Throughput::print_header() {
    std::cout << "[ ID]  Interval         Transfer      Bandwidth" << std::endl;
}

std::string Throughput::format_row(double start, double end, uint64_t bytes, double bps) {
    char buf[128];
    std::snprintf(buf, sizeof(buf), "[  0]  %5.2f-%-5.2f sec  %s  %s",
                  start, end,
                  format_bytes(bytes).c_str(),
                  format_bandwidth(bps).c_str());
    return std::string(buf);
}

void Throughput::report() const {
    // Emit a trailing partial bucket only if it represents a meaningful slice
    // of time (>= 1% of one interval). This avoids ugly slivers like
    // "10.00-10.02 sec" while still surfacing genuine partial buckets.
    if (interval_bytes_ > 0 && elapsed_ > interval_start_sec_) {
        double partial_elapsed = elapsed_ - interval_start_sec_;
        if (partial_elapsed >= interval_sec_ * 0.01) {
            double bps = (partial_elapsed > 0.0)
                ? (static_cast<double>(interval_bytes_) * 8.0) / partial_elapsed
                : 0.0;
            std::cout << format_row(interval_start_sec_, elapsed_,
                                    interval_bytes_, bps) << std::endl;
        }
    }

    std::cout << "- - - - - - - - - - - - - - - - - - -" << std::endl;
    std::cout << format_row(0.0, elapsed_, total_bytes_, get_bits_per_second()) << std::endl;
}
