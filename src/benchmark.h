#if !defined(BENCHMARK_H)
#define BENCHMARK_H

#include <chrono>

typedef std::chrono::time_point<std::chrono::high_resolution_clock> HighResClock;

struct Record {
    u64 time;
    u64 bytes;
};

internal HighResClock start_clock() {
    return std::chrono::high_resolution_clock::now();
}

internal i64 stop_clock(HighResClock *clock) {
    auto end_time = std::chrono::high_resolution_clock::now();

    auto start = std::chrono::time_point_cast<std::chrono::microseconds>(*clock).time_since_epoch().count();
    auto end = std::chrono::time_point_cast<std::chrono::microseconds>(end_time).time_since_epoch().count();

    auto duration = end - start;

    return duration;
}

#endif // BENCHMARK_H
