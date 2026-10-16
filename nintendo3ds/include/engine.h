#pragma once
#include <cstddef>
#include <cstdint>
#include <memory>

namespace port3ds {
enum class Scenario { empty, two_units, melee };
struct Snapshot {
    std::uint32_t frame = 0;
    unsigned units = 0;
    unsigned width = 0, height = 0;
};
struct UnitView {
    int x = 0, y = 0;
    unsigned owner = 0;
    bool selected = false;
};
// The platform UI never includes the engine's large implementation headers.
class Engine {
public:
    Engine();
    ~Engine();
    bool load(const char* data_root, const char* map_path, Scenario scenario);
    bool tick();
    Snapshot snapshot() const;
    const char* error() const;
    bool ready() const;
    bool select(int world_x, int world_y);
    bool move_selected(int world_x, int world_y);
    std::size_t visible_units(UnitView* output, std::size_t capacity) const;
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
bool core_self_test();
std::size_t state_size();
std::size_t unit_size();
}
