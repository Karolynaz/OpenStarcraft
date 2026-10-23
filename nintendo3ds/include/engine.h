#pragma once
#include <cstddef>
#include <cstdint>
#include <memory>
#include <array>
#include "cursor.h"
#include "skirmish.h"
#include "map_preview.h"

namespace port3ds {
enum class Scenario { empty, two_units, melee, ai_battle, versus_ai, gas_battle, terran_battle, terran_lab, protoss_battle, protoss_lab, zerg_battle, zerg_lab, protoss_ai_battle, protoss_versus_ai, zerg_ai_battle, zerg_versus_ai };
inline bool arena_scenario(Scenario s) {return s>=Scenario::terran_battle && s<=Scenario::zerg_lab;}
inline unsigned scenario_race(Scenario s) {
    return s==Scenario::zerg_battle || s==Scenario::zerg_lab || s==Scenario::zerg_ai_battle || s==Scenario::zerg_versus_ai?0:
        s==Scenario::protoss_battle || s==Scenario::protoss_lab || s==Scenario::protoss_ai_battle || s==Scenario::protoss_versus_ai?2:1;
}
enum class FogCell : unsigned { unexplored, explored, visible };
enum class Command { context, attack, stop, train, depot, barracks, cancel, primary, refinery };
enum class SelectionKind { none, workers, command_center, barracks, other };
enum class OptionKind : unsigned { build=1, addon, train, research, upgrade, order, stim, siege, cloak, liftoff, land, unload, cancel, merge, morph, burrow, larvae, nydus };
enum class OptionPage { primary, secondary, basic, advanced };
struct MenuOption { char label[64]{}; unsigned code=0,icon=~0u; bool target=false, enabled=false; };
inline unsigned option_code(OptionKind kind,unsigned id=0) {return (unsigned(kind)<<16)|id;}
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
    int selected_shields=0,selected_max_shields=0;
    unsigned queue = 0;
    unsigned selected = 0;
    int selected_energy=0,selected_max_energy=0,work_frames=0;
    bool selected_building=false,selected_larva=false;
    SelectionKind selection = SelectionKind::none;
    std::array<PlayerView,3> players{};
    int winner = -1;
    bool observer = false, benchmark = false;
    unsigned battle_seconds=0,end_reason=0,score[2]{};
};
struct SoundEvent { unsigned id = 0; int x = 0, y = 0; };
struct UnitView {
    int x = 0, y = 0;
    unsigned owner = 0;
    bool selected = false;
    unsigned type = 0;
    int hp = 0, max_hp = 0, width = 0, height = 0;
    bool completed = false, worker = false, mineral = false, geyser = false, refinery = false;
    bool flying=false; unsigned cargo=0;
    int shields=0,max_shields=0; unsigned fighters=0;
    unsigned order=0;int energy=0;bool burrowed=false;
};
struct SpriteInfo {unsigned width=0,height=0;bool valid=false;};
struct SpriteView {
    int x = 0, y = 0;
    unsigned image = 0, frame = 0, owner = 0;
    bool flipped = false, shadow = false;
    std::uint64_t order = 0;
    unsigned opacity=255;
    bool additive=false;
};
// The platform UI never includes the engine's large implementation headers.
class Engine {
public:
    static bool preview_map(const char* data_root,const char* map_path,MapPreview& output);
    Engine();
    ~Engine();
    bool load(const char* data_root, const char* map_path, Scenario scenario,const MatchSetup* setup=nullptr);
    bool tick();
    void advance_wall_time(std::uint64_t microseconds);
    unsigned options(OptionPage page,MenuOption* output,unsigned capacity) const;
    bool perform(unsigned code,int world_x=0,int world_y=0);
    PlacementView placement_unit(unsigned type,int x,int y) const;
    std::size_t drain_sounds(SoundEvent* output, std::size_t capacity);
    unsigned dropped_sounds() const;
    Snapshot snapshot() const;
    const char* error() const;
    bool ready() const;
    unsigned count_units(unsigned owner,unsigned type,bool completed=true) const;
    bool researched(unsigned tech) const;
    unsigned upgrade_level(unsigned upgrade) const;
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
    std::size_t remembered_units(UnitView* output,std::size_t capacity) const;
    std::uint32_t frame() const;
    bool fog_enabled() const;
    void set_fog_enabled(bool enabled);
    FogCell fog_cell(unsigned tile_x,unsigned tile_y) const;
    CursorKind cursor_kind(int world_x,int world_y,bool targeting=false) const;
    SpriteView command_icon(unsigned icon,bool enabled=true) const;
    SpriteView command_button() const;
    SpriteView cursor_image(CursorKind kind,std::uint64_t elapsed_us) const;
    SpriteInfo sprite_info(unsigned image,unsigned frame) const;
    bool sprite_pixels(unsigned image,unsigned frame,unsigned owner,std::uint16_t* output,std::size_t capacity) const;
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
