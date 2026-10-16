#pragma once
#include <cstdint>

namespace port3ds {
// Rational 24 Hz accumulator: no rounding to 41 ms, no dropped engine ticks.
// Pause/suspend time is explicitly excluded by the caller.
class Scheduler {
    std::uint64_t credit_ = 0;
public:
    void add(std::uint64_t elapsed_us) { credit_ += elapsed_us * 24; }
    bool due() const { return credit_ >= 1000000; }
    void consume() { if (due()) credit_ -= 1000000; }
    void reset() { credit_ = 0; }
    std::uint64_t backlog() const { return credit_ / 1000000; }
};
}
