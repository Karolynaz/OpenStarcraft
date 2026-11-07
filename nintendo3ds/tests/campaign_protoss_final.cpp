// Eye of the Storm. Use the owned CHK and public controls only.
#include "bwenums.h"
#include "engine.h"

#include <algorithm>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <vector>

using namespace port3ds;
using namespace bwgame;

namespace {
unsigned type(UnitTypes value) { return unsigned(value); }
int distance(const UnitView& unit, int x, int y) {
    return std::abs(unit.x - x) + std::abs(unit.y - y);
}
bool idle(const UnitView& unit) {
    return unit.order == unsigned(Orders::PlayerGuard) ||
           unit.order == unsigned(Orders::Guard) || unit.order == unsigned(Orders::Stop);
}
}

int main(int argc, char** argv) {
    assert(argc == 3);
    const auto scenario = Scenario::protoss_campaign_10;
    assert(campaign::index(unsigned(scenario)) == campaign::key_index("protoss10"));
    Engine engine;
    assert(engine.load(argv[1], argv[2], scenario));
    assert(engine.start_recording("/tmp/sc3ds-original-protoss-final-session"));
    engine.set_fog_enabled(false);

    const auto nexus = type(UnitTypes::Protoss_Nexus);
    const auto probe = type(UnitTypes::Protoss_Probe);
    const auto reaver = type(UnitTypes::Protoss_Reaver);
    const auto dragoon = type(UnitTypes::Protoss_Dragoon);
    const auto zealot = type(UnitTypes::Protoss_Zealot);
    const auto pylon = type(UnitTypes::Protoss_Pylon);
    const auto assimilator = type(UnitTypes::Protoss_Assimilator);
    const auto gateway = type(UnitTypes::Protoss_Gateway);
    const auto cannon = type(UnitTypes::Protoss_Photon_Cannon);
    const auto cybernetics = type(UnitTypes::Protoss_Cybernetics_Core);
    const auto robotics = type(UnitTypes::Protoss_Robotics_Facility);
    const auto support = type(UnitTypes::Protoss_Robotics_Support_Bay);
    const auto gantrithor = type(UnitTypes::Hero_Gantrithor);
    const auto hyperion = type(UnitTypes::Hero_Hyperion);
    const auto zeratul = type(UnitTypes::Hero_Zeratul);
    const auto dark_templar = type(UnitTypes::Hero_Dark_Templar);

    unsigned pending = 0, pending_at = 0, before = 0;
    int home_x = 0, home_y = 0;
    bool attack = false;
    for (unsigned frame = 0; frame < 120000 && engine.snapshot().winner < 0; ++frame) {
        assert(engine.tick());
        SoundEvent sounds[256];
        engine.drain_sounds(sounds, 256);
        if (frame % 48) continue;

        UnitView visible[1700];
        const auto count = engine.visible_units(visible, 1700);
        const auto state = engine.snapshot();
        std::vector<UnitView> workers, owned, minerals;
        const UnitView* overmind = nullptr;
        for (unsigned i = 0; i < count; ++i) {
            const auto& unit = visible[i];
            if (unit.owner == 0) {
                if (unit.type == probe && unit.completed) workers.push_back(unit);
                if (unit.type == nexus && unit.y > 5000) {
                    home_x = unit.x;
                    home_y = unit.y;
                }
                if (unit.completed) owned.push_back(unit);
            }
            if (unit.mineral) minerals.push_back(unit);
            if (unit.owner == 3 && unit.type == type(UnitTypes::Special_Overmind_With_Shell)) overmind = &unit;
        }
        if (!home_x) continue;

        const UnitView* expansion = nullptr;
        for (const auto& unit : owned)
            if (unit.type == nexus && unit.y < 4500) expansion = &unit;
        const unsigned army = engine.count_units(0, dragoon) + engine.count_units(0, reaver) +
                              engine.count_units(0, zealot);
        if (army >= 17 && engine.count_units(0, reaver) >= 3) attack = true;
        if (frame % 240 == 0) {
            for (const auto& unit : owned) {
                if (!engine.select(unit.x, unit.y)) continue;
                if (unit.type == hyperion && unit.y < 1700) {
                    if (unit.y > 340) engine.move_selected(6072, 80);
                } else if (unit.type == gantrithor) {
                    if (attack && overmind && overmind->hp < 3100 && unit.fighters >= 6 &&
                        unit.hp > 600 && unit.shields > 125) {
                        if (unit.y > 4680 && unit.x > 4100) engine.move_selected(3980, 4650);
                        else if (unit.x > 3200) engine.move_selected(3110, 3980);
                        else engine.command(Command::context, overmind->x, overmind->y);
                    } else if (unit.y < 5150) {
                        engine.move_selected(5480, 5504);
                    }
                } else if (unit.type == dark_templar || unit.type == zeratul) {
                    if (unit.type == dark_templar && attack && overmind)
                        engine.command(Command::attack, overmind->x, overmind->y);
                    else if (unit.y < 5020)
                        engine.move_selected(5290, 5550);
                } else if (attack && overmind &&
                           (unit.type == zealot || unit.type == dragoon || unit.type == reaver)) {
                    if (unit.x > 5000 && unit.y > 4350) engine.command(Command::attack, 5372, 4200);
                    else if (unit.x > 3580) engine.command(Command::attack, 3430, 3990);
                    else if (distance(unit, overmind->x, overmind->y) < 360)
                        engine.command(Command::context, overmind->x, overmind->y);
                    else engine.command(Command::attack, overmind->x, overmind->y);
                }
            }
        }

        for (const auto& worker : workers) {
            if (!idle(worker)) continue;
            const UnitView* target = nullptr;
            int best = std::numeric_limits<int>::max();
            for (const auto& mineral : minerals) {
                if (!expansion && mineral.y < 5100) continue;
                if (expansion && (mineral.y < 4400) != (worker.y < 4650)) continue;
                const int dx = mineral.x - worker.x, dy = mineral.y - worker.y;
                const int candidate = dx * dx + dy * dy;
                if (candidate < best) {
                    best = candidate;
                    target = &mineral;
                }
            }
            if (!target && expansion) {
                for (const auto& mineral : minerals) {
                    if (mineral.x > 5600 && mineral.y < 4000 && mineral.y > 3700) {
                        target = &mineral;
                        break;
                    }
                }
            }
            if (target && engine.select(worker.x, worker.y))
                engine.command(Command::context, target->x, target->y);
        }
        if (frame % 480 == 0 && workers.size() >= 9) {
            const UnitView* gas = nullptr;
            for (const auto& unit : owned)
                if (unit.type == assimilator) gas = &unit;
            if (gas) {
                for (unsigned i = 0; i < 3; ++i)
                    if (engine.select(workers[i].x, workers[i].y))
                        engine.command(Command::context, gas->x + 40, gas->y + 16);
            }
        }

        if (pending && (engine.count_units(0, pending) > before || frame > pending_at + 1500))
            pending = 0;
        unsigned wanted = 0;
        int anchor_x = home_x, anchor_y = home_y;
        for (const auto& unit : owned) {
            if (unit.type == gateway && unit.disabled) {
                wanted = pylon;
                anchor_x = unit.x;
                anchor_y = unit.y;
                break;
            }
        }
        if (!wanted && state.supply_max - state.supply < 8) wanted = pylon;
        if (!wanted && !engine.count_units(0, assimilator, false)) wanted = assimilator;
        if (!wanted && engine.count_units(0, pylon, false) < 3) wanted = pylon;
        if (!wanted && !engine.count_units(0, nexus, false)) continue;
        if (!wanted && engine.count_units(0, cannon, false) < 4) {
            wanted = cannon;
            anchor_x = 5344;
            anchor_y = 5488;
        }
        if (!wanted && !engine.count_units(0, gateway, false)) wanted = gateway;
        if (!wanted && !engine.count_units(0, cybernetics, false)) wanted = cybernetics;
        if (!wanted && engine.count_units(0, gateway, false) < 2) wanted = gateway;
        if (!wanted && !engine.count_units(0, robotics, false)) wanted = robotics;
        if (!wanted && !engine.count_units(0, support, false)) wanted = support;
        if (!wanted && army >= 8 && !expansion && engine.count_units(0, nexus, false) < 2 &&
            state.minerals >= 400) {
            wanted = nexus;
            anchor_x = 5760;
            anchor_y = 4072;
        }
        if ((wanted == pylon || wanted == cannon) && anchor_x == home_x) {
            anchor_x = home_x - 192;
            anchor_y = home_y - 160;
        }
        if (!pending && wanted && !workers.empty()) {
            auto builder = workers.back();
            if (wanted == nexus) {
                builder = *std::min_element(workers.begin(), workers.end(), [&](const UnitView& a, const UnitView& b) {
                    return distance(a, anchor_x, anchor_y) < distance(b, anchor_x, anchor_y);
                });
            }
            bool placed = false;
            if (engine.select(builder.x, builder.y)) {
                auto build_at = [&](int x, int y) {
                    return engine.placement_unit(wanted, x, y).valid &&
                           engine.perform(option_code(OptionKind::build, wanted), x, y);
                };
                if (wanted == assimilator) {
                    for (unsigned i = 0; i < count && !placed; ++i)
                        if (visible[i].geyser && distance(visible[i], home_x, home_y) < 2400)
                            placed = build_at(visible[i].x, visible[i].y);
                } else {
                    for (int ring = 2; ring < 11 && !placed; ++ring)
                        for (int y = -ring; y <= ring && !placed; ++y)
                            for (int x = -ring; x <= ring && !placed; ++x)
                                if (std::abs(x) == ring || std::abs(y) == ring)
                                    placed = build_at(anchor_x + x * 32, anchor_y + y * 32);
                }
            }
            if (placed) {
                pending = wanted;
                pending_at = frame;
                before = engine.count_units(0, wanted);
            }
        }
        for (const auto& unit : owned) {
            if (!engine.select(unit.x, unit.y) || engine.snapshot().queue) continue;
            const auto train = [&](unsigned product) {
                return engine.perform(option_code(OptionKind::train, product));
            };
            if (unit.type == gantrithor && unit.fighters < 8)
                train(type(UnitTypes::Protoss_Interceptor));
            else if (unit.type == nexus && engine.count_units(0, probe) < 26 &&
                     (expansion || engine.count_units(0, probe) < 16))
                train(probe);
            else if (unit.type == robotics && engine.count_units(0, reaver) < 6)
                train(reaver);
            else if (unit.type == reaver && unit.fighters < 5)
                train(type(UnitTypes::Protoss_Scarab));
            else if (unit.type == gateway && army < 28 && state.minerals > 250) {
                const auto product = engine.count_units(0, dragoon) < 16 ? dragoon : zealot;
                if (!train(product) && product == dragoon) train(zealot);
            }
        }
    }

    assert(attack && engine.snapshot().winner == 0);
    assert(engine.count_units(0, zeratul) == 1);
    assert(engine.count_units(0, gantrithor) == 1);
    assert(engine.count_units(0, hyperion) == 1);
    assert(engine.count_units(3, type(UnitTypes::Special_Overmind_With_Shell)) == 0);
    const char* path = "/tmp/sc3ds-protoss10-completed.sav";
    assert(engine.save_game(path, SaveView{}));
    Engine replay;
    SaveView restored;
    assert(replay.load_save(argv[1], path, "/tmp/sc3ds-original-protoss-final-replay",
                            "/tmp/sc3ds-original-protoss-final-map", restored));
    assert(replay.state_digest() == engine.state_digest());
    std::printf("Eye of the Storm: original victory and exact replay at frame %u PASS\n", engine.frame());
}
