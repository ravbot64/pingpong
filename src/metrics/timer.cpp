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
    return (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
}
