#include "throughput.h"
#include "../common/units.h"

#include <iostream>
#include <cstdio>

void Throughput::start() {
    total_bytes_ = 0;
    elapsed_ = 0.0;
    timer_.start();
}

void Throughput::add_bytes(size_t count) {
    total_bytes_ += count;
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

void Throughput::report() const {
    std::cout << "- - - - - - - - - - - - - - - - - - -" << std::endl;
    std::cout << "[ ID]  Interval         Transfer      Bandwidth" << std::endl;

    char interval[32];
    std::snprintf(interval, sizeof(interval), "0.00-%.2f sec", elapsed_);

    std::cout << "[  0]  "
              << interval << "   "
              << format_bytes(total_bytes_) << "   "
              << format_bandwidth(get_bits_per_second())
              << std::endl;
}
