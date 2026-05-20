#include "throughput.h"
#include "../common/units.h"

#include <iostream>
#include <cstdio>

void Throughput::set_interval(double seconds) {
    interval_sec_ = seconds;
}

void Throughput::start() {
    total_bytes_ = 0;
    elapsed_ = 0.0;
    interval_bytes_ = 0;
    interval_start_ = 0.0;

    timer_.start();
    interval_timer_.start();
}

void Throughput::add_bytes(size_t count) {
    total_bytes_ += count;
    interval_bytes_ += count;
}

bool Throughput::check_interval(std::string& line) {
    double interval_elapsed = interval_timer_.elapsed_seconds();
    if (interval_elapsed < interval_sec_) {
        return false;
    }

    // Interval has elapsed — compute stats and format line
    double interval_end = interval_start_ + interval_elapsed;
    double bps = (interval_elapsed > 0.0)
        ? (static_cast<double>(interval_bytes_) * 8.0) / interval_elapsed
        : 0.0;

    line = format_row(interval_start_, interval_end, interval_bytes_, bps);

    // Reset for next interval
    interval_start_ = interval_end;
    interval_bytes_ = 0;
    interval_timer_.reset();

    return true;
}

void Throughput::stop() {
    elapsed_ = timer_.elapsed_seconds();
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
    // Flush any remaining partial interval as a final row
    if (interval_bytes_ > 0 && elapsed_ > interval_start_) {
        double partial_elapsed = elapsed_ - interval_start_;
        double bps = (partial_elapsed > 0.0)
            ? (static_cast<double>(interval_bytes_) * 8.0) / partial_elapsed
            : 0.0;
        std::cout << format_row(interval_start_, elapsed_, interval_bytes_, bps) << std::endl;
    }

    // Separator + aggregate summary
    std::cout << "- - - - - - - - - - - - - - - - - - -" << std::endl;
    std::cout << format_row(0.0, elapsed_, total_bytes_, get_bits_per_second()) << std::endl;
}
