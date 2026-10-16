#include "engine.h"
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <string>

int main(int argc, char** argv) {
    if (!port3ds::core_self_test()) { std::fprintf(stderr, "Core self test FAILED\n"); return 1; }
    std::printf("Core DCL/endian/fixed-point PASS; sizeof state=%zu unit=%zu\n",
                port3ds::state_size(), port3ds::unit_size());
    if (argc == 2 && std::string(argv[1]) == "--self-test") return 0;
    if (argc < 3) {
        std::fprintf(stderr, "Usage: host-sim DATA_ROOT MAP [empty|two|melee] [ticks]\n"); return 2;
    }
    port3ds::Scenario scenario = port3ds::Scenario::empty;
    if (argc > 3) {
        const std::string name(argv[3]);
        if (name == "two") scenario = port3ds::Scenario::two_units;
        else if (name == "melee") scenario = port3ds::Scenario::melee;
        else if (name != "empty") { std::fprintf(stderr, "Unknown scenario\n"); return 2; }
    }
    const int count = argc > 4 ? std::atoi(argv[4]) : 240;
    if (count <= 0 || count > 1000000) return 2;
    using clock = std::chrono::steady_clock;
    const auto start = clock::now();
    port3ds::Engine engine;
    if (!engine.load(argv[1], argv[2], scenario)) {
        std::fprintf(stderr, "Load failed: %s\n", engine.error()); return 1;
    }
    const auto loaded = clock::now();
    double sum = 0, worst = 0;
    for (int i = 0; i != count; ++i) {
        auto begin = clock::now();
        if (!engine.tick()) { std::fprintf(stderr, "Tick failed: %s\n", engine.error()); return 1; }
        double us = std::chrono::duration<double, std::micro>(clock::now() - begin).count();
        sum += us; if (us > worst) worst = us;
    }
    auto s = engine.snapshot();
    std::printf("platform,map_width,map_height,frame,units,load_ms,tick_mean_us,tick_max_us\n"
                "host,%u,%u,%u,%u,%.2f,%.2f,%.2f\n", s.width, s.height, s.frame, s.units,
                std::chrono::duration<double, std::milli>(loaded - start).count(), sum / count, worst);
    std::puts("Host results do not predict original 3DS performance.");
    return 0;
}
