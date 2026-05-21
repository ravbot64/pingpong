#include "timer.h"

void Timer::start(double duration_sec) {
    duration_sec_ = duration_sec;
    start_time_ = now();
}

bool Timer::is_expired() const {
    if (duration_sec_ <= 0.0) return false;
    return elapsed_seconds() >= duration_sec_;
}

double Timer::elapsed_seconds() const {
    return diff_seconds(start_time_, now());
}

void Timer::reset() {
    start_time_ = now();
}

struct timespec Timer::now() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts;
}

double Timer::diff_seconds(const struct timespec& start, const struct timespec& end) {
    double sec_delta = static_cast<double>(end.tv_sec - start.tv_sec);
    double nsec_delta = static_cast<double>(end.tv_nsec - start.tv_nsec);
    return sec_delta + nsec_delta / 1e9;
}
