#include "scheduler.h"
#include <cassert>
#include <cstdio>
int main() {
    port3ds::Scheduler s;
    unsigned ticks = 0;
    for (unsigned i = 0; i != 1000; ++i) {
        s.add(1000);
        while (s.due()) { s.consume(); ++ticks; }
    }
    assert(ticks == 24 && s.backlog() == 0);
    s.add(1000000);
    for (int i = 0; i != 2; ++i) s.consume();
    assert(s.backlog() == 22); // Slow frames retain debt.
    s.reset(); assert(!s.due());
    s.add(41666); assert(!s.due());
    s.add(1); assert(s.due());
    std::puts("24 Hz scheduler / backlog / pause reset PASS");
}
