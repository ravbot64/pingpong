#ifndef PINGPONG_TIMER_H
#define PINGPONG_TIMER_H

#include <ctime>  // timespec, clock_gettime

class Timer {
public:
    // Start the timer with an optional duration limit (0 = no limit, just measure elapsed)
    void start(double duration_sec = 0.0);

    // Returns true if duration has been exceeded
    bool is_expired() const;

    // Returns elapsed time in seconds since start()
    double elapsed_seconds() const;

    // Reset start time to now (useful for interval reporting in Phase 2)
    void reset();

private:
    struct timespec start_time_{};
    double duration_sec_ = 0.0;

    // Helper: get current monotonic time
    static struct timespec now();

    // Helper: compute difference in seconds between two timespecs
    static double diff_seconds(const struct timespec& start, const struct timespec& end);
};

#endif
