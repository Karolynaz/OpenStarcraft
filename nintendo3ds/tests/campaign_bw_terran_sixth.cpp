#include "engine.h"
#include "bwenums.h"
#include <cassert>
#include <cstdio>
#include <string>

using namespace port3ds;
using namespace bwgame;

int main(int argc, char** argv) {
    assert(argc == 4);
    const std::string journal = argv[3];
    Engine engine;
    assert(engine.load(argv[1], argv[2], Scenario::bw_terran_campaign_6));

    assert(engine.start_recording(journal.c_str()));
    engine.set_fog_enabled(false);
    bool saw_airborne_command_center = false;
    bool launched = false, unloaded = false;
    unsigned waypoint = 0;
    constexpr unsigned dropship = unsigned(UnitTypes::Terran_Dropship);
    constexpr unsigned tank = unsigned(UnitTypes::Terran_Siege_Tank_Tank_Mode);
    constexpr unsigned siege_tank = unsigned(UnitTypes::Terran_Siege_Tank_Siege_Mode);
    struct Point { int x, y; };
    // Fly around the Zerg positions and unload on the north side of Raynor's base.
    constexpr Point path[] = {{928,3040}, {2528,3040}, {3104,2464}, {3296,2464},
        {3360,2400}, {3424,2400}, {3808,2016}, {3808,1952}, {3872,1888},
        {3872,1440}, {4064,1248}, {4064,352}, {3808,96}, {3680,70}};
    const auto near = [](const UnitView& unit, Point point, int radius) {
        const int dx = unit.x-point.x, dy = unit.y-point.y;
        return dx*dx+dy*dy < radius*radius;
    };
    for (unsigned frame = 0; frame < 10000 && engine.snapshot().winner < 0; ++frame) {
        assert(engine.tick());
        if (frame % 12) continue;
        UnitView units[1700];
        const auto count = engine.visible_units(units, 1700);
        UnitView ship{};
        for (std::size_t i = 0; i < count; ++i) {
            const auto& unit = units[i];
            if (unit.owner == 2 && unit.type == unsigned(UnitTypes::Terran_Command_Center) && unit.flying)
                saw_airborne_command_center = true;
            if (unit.owner == 0 && unit.type == dropship &&
                (!ship.hp || unit.cargo > ship.cargo || (!unit.cargo && !ship.cargo && unit.x < ship.x)))
                ship = unit;
        }
        if (frame == 2400) {
            assert(saw_airborne_command_center);
            assert(engine.count_units(0, unsigned(UnitTypes::Terran_Command_Center)) == 1);
            assert(engine.count_units(0, dropship) == 2);
            assert(engine.count_units(0, unsigned(UnitTypes::Terran_Science_Vessel)) == 2);
            assert(engine.count_units(0, unsigned(UnitTypes::Terran_SCV)) == 6);
        }
        if (frame < 2300) continue;
        if (frame % 120 == 0) {
            for (std::size_t i = 0; i < count; ++i) {
                const auto& unit = units[i];
                if (unit.owner || !engine.select(unit.x, unit.y)) continue;
                if (unit.type == unsigned(UnitTypes::Terran_Factory) &&
                    !engine.count_units(0, unsigned(UnitTypes::Terran_Machine_Shop), false))
                    engine.perform(option_code(OptionKind::addon, unsigned(UnitTypes::Terran_Machine_Shop)));
            }
        }
        if (frame < 2470) {
            // Make the transports selectable where their scripted passengers exited.
            for (std::size_t i = 0; i < count; ++i) {
                const auto& unit = units[i];
                if (unit.owner || (unit.type != unsigned(UnitTypes::Terran_Marine) && !unit.worker) ||
                    unit.x >= 1050 || unit.y <= 3400 || !engine.select(unit.x, unit.y)) continue;
                const auto selected = engine.snapshot();
                if (!selected.selected_building && selected.selected_max_hp <= 60)
                    engine.move_selected(unit.x+120, unit.y+170);
            }
        }
        if (!launched && ship.hp) {
            if (ship.cargo < 2 && frame % 60 == 0 && engine.select(ship.x, ship.y)) {
                for (std::size_t i = 0; i < count; ++i) {
                    const auto& unit = units[i];
                    if (!unit.owner && unit.type == tank && unit.x < 1400 &&
                        engine.perform(option_code(OptionKind::order, unsigned(Orders::PickupTransport)),
                                       unit.x, unit.y)) break;
                }
            } else if (ship.cargo >= 2 && engine.select(ship.x, ship.y)) {
                assert(engine.researched(unsigned(TechTypes::Tank_Siege_Mode)));
                launched = engine.move_selected(path[0].x, path[0].y);
            }
        }
        if (launched && !unloaded && ship.cargo && engine.select(ship.x, ship.y)) {
            if (near(ship, path[waypoint], 32)) {
                if (waypoint+1 < sizeof path / sizeof path[0]) {
                    ++waypoint;
                    engine.move_selected(path[waypoint].x, path[waypoint].y);
                } else {
                    unloaded = engine.perform(option_code(OptionKind::unload));
                }
            } else if (ship.order == unsigned(Orders::PickupIdle)) {
                engine.move_selected(path[waypoint].x, path[waypoint].y);
            }
        }
        if (unloaded && frame % 48 == 0) {
            constexpr Point objective{3392,208};
            for (std::size_t i = 0; i < count; ++i) {
                const auto& unit = units[i];
                if (unit.owner || (unit.type != tank && unit.type != siege_tank) ||
                    unit.y > 850 || !engine.select(unit.x, unit.y)) continue;
                if (unit.type == tank && near(unit, objective, 380))
                    engine.perform(option_code(OptionKind::siege));
                else
                    engine.command(Command::context, objective.x, objective.y);
            }
        }
    }
    assert(launched && unloaded && engine.snapshot().winner == 0);
    assert(engine.count_units(7, unsigned(UnitTypes::Terran_Command_Center)) == 0);
    const auto save = journal + ".ob3";
    assert(engine.save_game(save.c_str(), SaveView{}));

    Engine replay;
    SaveView view;
    assert(replay.load_save(argv[1], save.c_str(), (journal+"-replay").c_str(),
                            (journal+"-map").c_str(), view));
    assert(replay.snapshot().winner == 0 && replay.state_digest() == engine.state_digest());
    printf("Emperor's Flight: authored base arrival, two-tank drop, victory and exact replay at frame %u PASS\n",
           engine.frame());
}
