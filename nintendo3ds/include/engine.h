#pragma once
#include <cstddef>
#include <cstdint>
#include <memory>
#include <array>

namespace port3ds {
enum class Scenario { empty, two_units, melee, ai_battle, versus_ai };
enum class Command { context, attack, stop, train, depot, barracks, cancel, primary };
enum class SelectionKind { none, workers, command_center, barracks, other };
struct PlayerView {
    int minerals = 0, gas = 0, supply = 0, supply_max = 0;
    unsigned workers = 0, army = 0, buildings = 0, live = 0;
    unsigned trained = 0, built = 0, waves = 0, lost = 0;
    bool ai = false;
};
struct PlacementView { int x = 0, y = 0, width = 0, height = 0; bool valid = false; };
struct Snapshot {
    std::uint32_t frame = 0;
    unsigned units = 0;
    unsigned width = 0, height = 0;
    int minerals = 0, gas = 0, supply = 0, supply_max = 0;
    const char* selected_name = "No selection";
    int selected_hp = 0, selected_max_hp = 0;
    unsigned queue = 0;
    unsigned selected = 0;
    SelectionKind selection = SelectionKind::none;
    std::array<PlayerView,2> players{};
    int winner = -1;
    bool observer = false;
};
struct UnitView {
    int x = 0, y = 0;
    unsigned owner = 0;
    bool selected = false;
    unsigned type = 0;
    int hp = 0, max_hp = 0, width = 0, height = 0;
    bool completed = false, worker = false, mineral = false;
};
struct SpriteView {
    int x = 0, y = 0;
    unsigned image = 0, frame = 0, owner = 0;
    bool flipped = false, shadow = false;
    std::uint64_t order = 0;
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
    bool activate(int world_x, int world_y);
    void clear_selection();
    unsigned select_army(int camera_x, int camera_y);
    unsigned select_workers();
    bool store_group(unsigned slot);
    bool recall_group(unsigned slot, int* center_x = nullptr, int* center_y = nullptr);
    bool observer_focus(int& x, int& y) const;
    bool move_selected(int world_x, int world_y);
    bool command(Command command, int world_x = 0, int world_y = 0);
    const char* command_status() const;
    PlacementView placement(Command building, int world_x, int world_y) const;
    std::size_t visible_units(UnitView* output, std::size_t capacity) const;
    std::size_t visible_images(SpriteView* output, std::size_t capacity, int camera_x, int camera_y) const;
    std::size_t image_types(unsigned* output, std::size_t capacity) const;
    std::uint32_t terrain_signature() const;
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
bool core_self_test();
std::size_t state_size();
std::size_t unit_size();
}
