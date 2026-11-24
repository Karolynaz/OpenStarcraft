// Original owned Return to Char map, with normal fog and only public game commands.
#include "bwenums.h"
#include "engine.h"
#include "sound_ids.h"
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

using namespace bwgame;
using namespace port3ds;

namespace {
struct Point { int x, y; };

// The fleet takes the inland route, then approaches the Khalis crystal from the west.
// Waypoints were recorded in a separate navigation run on the same map.
constexpr Point flight_path[] = {
    {2512,272},{2512,688},{2992,1168},{2992,1200},{3024,1232},{3024,1264},
    {3056,1296},{3056,1552},{2928,1680},{2928,1712},{2896,1744},{2896,1776},
    {2992,1872},{2992,2000},{2960,2032},{2960,2064},{2800,2224},{2800,2288},
    {2768,2320},{2768,2480},{2800,2512},{2800,2576},{2992,2768},{3024,2768},
    {3056,2800},{3056,2832},{3120,2896},{3120,2928},{3152,2960},{3152,2992},
    {3216,3056},{3216,3088},{3408,3280},{3440,3280},{3504,3344},{3696,3344},
    {3728,3376},{3757,3420},
};

constexpr unsigned probe = unsigned(UnitTypes::Protoss_Probe);
constexpr unsigned nexus = unsigned(UnitTypes::Protoss_Nexus);
constexpr unsigned shuttle = unsigned(UnitTypes::Protoss_Shuttle);
constexpr unsigned pylon = unsigned(UnitTypes::Protoss_Pylon);
constexpr unsigned core = unsigned(UnitTypes::Protoss_Cybernetics_Core);
constexpr unsigned robotics = unsigned(UnitTypes::Protoss_Robotics_Facility);
constexpr unsigned support = unsigned(UnitTypes::Protoss_Robotics_Support_Bay);
constexpr unsigned speed = unsigned(UpgradeTypes::Gravitic_Drive);

void prepare_fleet(Engine& game, unsigned tick, const Snapshot& state,
                   const UnitView* units, unsigned count,
                   const std::vector<UnitView>& workers, const UnitView& transport) {
    if (workers.empty()) return;
    unsigned building = 0;
    if (!game.count_units(0, core, false)) building = core;
    else if (!game.count_units(0, robotics, false)) building = robotics;
    else if (!game.count_units(0, support, false)) building = support;
    else if (state.supply_max - state.supply < 8 && game.count_units(0, pylon, false) < 8)
        building = pylon;

    const auto& courier = workers.front();
    if (tick % 96 == 0 && building && state.minerals >= 200 &&
        (building != robotics || state.gas >= 100) && game.select(courier.x, courier.y)) {
        for (unsigned i = 0; i < 19 * 19; ++i) {
            const int x = 2260 + int(i % 19) * 32;
            const int y = 110 + int(i / 19) * 32;
            if (game.placement_unit(building, x, y).valid &&
                game.perform(option_code(OptionKind::build, building), x, y))
                break;
        }
    }
    for (unsigned i = 0; i < count; ++i) {
        const auto& unit = units[i];
        if (unit.owner || !unit.completed || !game.select(unit.x, unit.y)) continue;
        if (unit.type == nexus && workers.size() < 12 && !game.selection_info().queue_count)
            game.perform(option_code(OptionKind::train, probe));
        if (unit.type == support && !game.upgrade_level(speed))
            game.perform(option_code(OptionKind::upgrade, speed));
        if (unit.type == robotics && game.count_units(0, shuttle, false) < 9 &&
            !game.selection_info().queue_count)
            game.perform(option_code(OptionKind::train, shuttle));
    }
    if (tick % 48 == 0) for (const auto& worker : workers) {
        const auto order = Orders(worker.order);
        if (order != Orders::Guard && order != Orders::PlayerGuard && order != Orders::Stop)
            continue;
        UnitView mineral{};
        int nearest = 800 * 800;
        for (unsigned i = 0; i < count; ++i) {
            const auto& unit = units[i];
            if (!unit.mineral) continue;
            const int dx = worker.x - unit.x, dy = worker.y - unit.y;
            const int distance = dx * dx + dy * dy;
            if (distance < nearest) { mineral = unit; nearest = distance; }
        }
        if (mineral.hp && game.select(worker.x, worker.y))
            game.command(Command::context, mineral.x, mineral.y);
    }
    if (transport.hp && !transport.cargo && game.select(courier.x, courier.y))
        game.command(Command::context, transport.x, transport.y);
}
}

int main(int argc, char** argv) {
    assert(argc == 4);
    const std::string session = argv[3];
    Engine game;
    assert(game.load(argv[1], argv[2], Scenario::bw_protoss_campaign_6));
    assert(game.start_recording(session.c_str()) && game.fog_enabled());
    const auto waypoints = sizeof flight_path / sizeof flight_path[0];
    unsigned waypoint = 0, voices = 0;
    bool departed = false, unload_ordered = false, reached_crystal = false;
    for (unsigned tick = 0; tick < 35000 && game.snapshot().winner < 0; ++tick) {
        if (!game.tick()) {
            std::fprintf(stderr, "Return to Char frame %u: %s\n", game.frame(), game.error());
            return 1;
        }
        SoundEvent sounds[256];
        const auto sound_count = game.drain_sounds(sounds, 256);
        for (unsigned i = 0; i < sound_count; ++i)
            if (sounds[i].id >= campaign_sound_base) ++voices;
        if (tick % 24) continue;

        UnitView units[1700], transport{};
        const auto count = game.visible_units(units, 1700);
        std::vector<UnitView> workers;
        for (unsigned i = 0; i < count; ++i) {
            const auto& unit = units[i];
            if (unit.owner) continue;
            if (unit.type == probe && unit.y < 800) workers.push_back(unit);
            if (unit.type == shuttle && (unit.cargo > transport.cargo || !transport.hp))
                transport = unit;
        }
        if (!departed) {
            prepare_fleet(game, tick, game.snapshot(), units, count, workers, transport);
            if (transport.cargo && game.count_units(0, shuttle) >= 9) {
                departed = true;
                std::printf("Return to Char fleet departed at frame %u\n", game.frame());
            }
        }
        if (departed && !unload_ordered && transport.hp && transport.cargo &&
            game.select(transport.x, transport.y)) {
            auto target = flight_path[waypoint];
            if (waypoint + 1 == waypoints) {
                const auto order = option_code(OptionKind::order, unsigned(Orders::MoveUnload));
                unload_ordered = game.perform(order, target.x, target.y);
            } else {
                if (std::abs(transport.x - target.x) + std::abs(transport.y - target.y) < 38)
                    target = flight_path[++waypoint];
                game.move_selected(target.x, target.y);
                for (unsigned i = 0; i < count; ++i) {
                    const auto& decoy = units[i];
                    if (!decoy.owner && decoy.type == shuttle && !decoy.cargo &&
                        game.select(decoy.x, decoy.y))
                        game.move_selected(target.x, target.y);
                }
            }
        }
        if (unload_ordered) for (unsigned i = 0; i < count; ++i) {
            const auto& unit = units[i];
            if (unit.owner || unit.type != probe || unit.y <= 3000) continue;
            // The engine evaluates Bring every tick against unit bounds.
            // These 24-tick control samples only check the Probe reached the Khalis crystal grounds.
            if (std::abs(unit.x - 3728) < 96 && std::abs(unit.y - 3456) < 80)
                reached_crystal = true;
            if (game.select(unit.x, unit.y)) game.move_selected(3728, 3456);
        }
    }
    assert(departed && unload_ordered && reached_crystal);
    assert(game.fog_enabled() && game.snapshot().winner == 0 && voices >= 2);
    const auto save = session + ".ob3";
    assert(game.save_game(save.c_str(), SaveView{}));
    Engine replay;
    SaveView view;
    assert(replay.load_save(argv[1], save.c_str(), (session + "-replay").c_str(),
                            (session + "-map").c_str(), view));
    assert(replay.snapshot().winner == 0 && replay.state_digest() == game.state_digest());
    std::printf("Return to Char: Khalis crystal recovery, %u voice events, victory at frame %u and exact replay PASS\n",
                voices, game.frame());
}
