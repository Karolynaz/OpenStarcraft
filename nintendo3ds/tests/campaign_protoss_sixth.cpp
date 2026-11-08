// Into the Darkness, driven through the same commands as the console.
#include "bwenums.h"
#include "engine.h"
#include "result_screen.h"

#include <algorithm>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <string>

using namespace port3ds;
using namespace bwgame;

namespace {
constexpr unsigned tassadar = unsigned(UnitTypes::Hero_Tassadar);
constexpr unsigned zeratul = unsigned(UnitTypes::Hero_Zeratul);
constexpr unsigned marine = unsigned(UnitTypes::Terran_Marine);
constexpr unsigned bomber = unsigned(UnitTypes::Zerg_Infested_Terran);
constexpr unsigned first_building = unsigned(UnitTypes::Terran_Command_Center);

struct Point { int x, y; };

int distance_squared(const UnitView& a, const UnitView& b) {
    const int dx = a.x - b.x, dy = a.y - b.y;
    return dx * dx + dy * dy;
}

bool hostile_troop(const UnitView& unit) {
    return (unit.owner == 3 || unit.owner == 6) && unit.type < first_building;
}

bool order(Engine& engine, Orders command, int x = 0, int y = 0) {
    return engine.perform(option_code(OptionKind::order, unsigned(command)), x, y);
}
}

int main(int argc, char** argv) {
    assert(argc == 4 || argc == 5);
    const std::string output = argv[3];
    const bool defeat = argc == 5 && std::string(argv[4]) == "defeat";
    assert(argc == 4 || defeat);
    const auto scenario = Scenario::protoss_campaign_6;
    Engine engine;
    assert(engine.load(argv[1], argv[2], scenario));
    assert(engine.start_recording((output + "-journal").c_str()));
    // The host pilot uses map-wide observations to pick normal orders.
    engine.set_fog_enabled(false);

    const Point waypoints[] = {{2064, 992}, {2640, 512}, {512, 1440},
                               {560, 2112}, {3770, 3378}, {2992, 3792}};
    unsigned stage = 0, hero_id = 0, reserve_id = 0;
    unsigned spell_at = 0, decoy_at = 0, ambush_at = 0;
    int previous_energy = 0;
    bool casting = false, resting = false;
    Point rest_point = {224, 192};

    for (unsigned frame = 0; frame < 120000 && engine.snapshot().winner < 0; ++frame) {
        assert(engine.tick());
        SoundEvent sounds[256];
        engine.drain_sounds(sounds, 256);
        if (frame % 6) continue;

        UnitView units[1700];
        const auto count = engine.visible_units(units, 1700);
        const UnitView* hero = nullptr;
        for (size_t i = 0; i < count; ++i) {
            const auto& unit = units[i];
            if (!unit.owner && unit.type == tassadar &&
                (!hero_id || engine.own_unit_at(unit.x, unit.y) == hero_id))
                hero = &unit;
        }
        if (!hero) {
            for (unsigned wait = 0; wait < 240 && engine.snapshot().winner < 0; ++wait)
                assert(engine.tick());
            break;
        }
        if (!hero_id) {
            hero_id = engine.own_unit_at(hero->x, hero->y);
            assert(hero_id && engine.select(hero->x, hero->y));
            engine.store_group(3);
        }

        Point goal = waypoints[stage];
        const UnitView* reserve = nullptr;
        for (size_t i = 0; i < count && stage < 5; ++i) {
            const auto& unit = units[i];
            if (unit.owner || unit.type >= first_building) continue;
            const bool reached = stage == 4
                ? unit.x >= 3683 && unit.x <= 3773 && unit.y >= 3331 && unit.y <= 3390
                : std::abs(unit.x - goal.x) + std::abs(unit.y - goal.y) < 80;
            if (reached) { goal = waypoints[++stage]; break; }
        }
        for (size_t i = 0; i < count; ++i) {
            const auto& unit = units[i];
            if (unit.owner || unit.type >= first_building) continue;
            if (stage == 4 && !ambush_at && unit.x > 1472 && unit.x < 1760 && unit.y > 3744) {
                ambush_at = frame;
                printf("Southern ambush triggered at frame %u\n", frame);
            }
            if (unit.type != marine) continue;
            if (!reserve_id && stage <= 2 && unit.hp >= 35)
                reserve_id = engine.own_unit_at(unit.x, unit.y);
            if (reserve_id && engine.own_unit_at(unit.x, unit.y) == reserve_id)
                reserve = &unit;
        }

        // Preserve one rescued Marine to create expendable scouts for the southern passage.
        if (stage == 4 && reserve && hero->x > 1040 && hero->y > 3650 && !ambush_at) {
            if (frame % 120 == 0 && engine.select(reserve->x, reserve->y))
                order(engine, Orders::AttackMove, hero->x - 45, hero->y - 45);
            if (!decoy_at && hero->energy >= 100 && distance_squared(*hero, *reserve) < 160 * 160 &&
                engine.recall_group(3) && order(engine, Orders::CastHallucination, reserve->x, reserve->y))
                decoy_at = frame;
        }
        if (stage == 4 && decoy_at && (!ambush_at || frame - ambush_at < 2400) && frame % 24 == 0) {
            for (size_t i = 0; i < count; ++i) {
                const auto& unit = units[i];
                if (!unit.owner && unit.type == marine && &unit != reserve && engine.select(unit.x, unit.y))
                    engine.move_selected(1900, 3860);
            }
        }
        if (stage == 4 && decoy_at && frame > decoy_at + 24 &&
            (!ambush_at || frame - ambush_at < 2100)) {
            if (casting && (hero->energy < previous_energy - 45 || frame > spell_at + 24)) casting = false;
            if (!casting && frame % 12 == 0 && engine.recall_group(3))
                engine.move_selected(defeat ? 1900 : 650, defeat ? 3860 : 2112);
            continue;
        }
        // Touch the switch from the far side of the beacon and leave the firing lane.
        if (stage == 5 || (stage == 4 && ambush_at && frame - ambush_at >= 2100 && hero->x > 3150)) {
            if (frame % 12 == 0 && engine.recall_group(3)) engine.move_selected(goal.x, goal.y);
            continue;
        }

        const UnitView* infested = nullptr;
        int infested_range = 560 * 560;
        if (stage >= 4 && hero->y >= 2450) {
            for (size_t i = 0; i < count; ++i) {
                const auto& unit = units[i];
                const int distance = distance_squared(unit, *hero);
                if (hostile_troop(unit) && unit.type == bomber && distance < infested_range) {
                    infested = &unit;
                    infested_range = distance;
                }
            }
        }
        if (infested && frame % 24 == 0) {
            for (size_t i = 0; i < count; ++i) {
                const auto& unit = units[i];
                if (!unit.owner && (unit.type == bomber || (unit.type == tassadar && &unit != hero)) &&
                    engine.select(unit.x, unit.y))
                    engine.move_selected(infested->x, infested->y);
            }
        }
        if (!ambush_at && infested && !casting && !resting && hero->energy >= 100 &&
            frame > spell_at + 110 && engine.recall_group(3)) {
            const UnitView* target = infested;
            if (infested_range > 370 * 370) {
                for (size_t i = 0; i < count; ++i) {
                    const auto& unit = units[i];
                    if (!unit.owner && &unit != hero && unit.type < first_building &&
                        distance_squared(unit, *hero) < 300 * 300) { target = &unit; break; }
                }
            }
            casting = order(engine, Orders::CastHallucination, target->x, target->y);
            if (casting) { spell_at = frame; previous_energy = hero->energy; }
        }
        if (casting && (hero->energy < previous_energy - 45 || frame > spell_at + 260)) {
            casting = false;
            if (!ambush_at && engine.select(hero->x, hero->y)) order(engine, Orders::HoldPosition);
            spell_at = frame;
        }
        if ((!ambush_at || frame - ambush_at >= 1600) && (!decoy_at || frame > decoy_at + 2000) &&
            !casting && frame > spell_at + 24 && hero->energy >= 75 &&
            (!infested || infested_range < 235 * 235)) {
            const UnitView* target = nullptr;
            int best = -1;
            for (size_t i = 0; i < count; ++i) {
                const auto& unit = units[i];
                const int range = unit.type == bomber ? 300 : 250;
                if (!hostile_troop(unit) || distance_squared(unit, *hero) > range * range) continue;
                int score = unit.type == bomber ? 100 : 0;
                for (size_t j = 0; j < count; ++j)
                    if (units[j].owner == unit.owner &&
                        std::abs(units[j].x - unit.x) + std::abs(units[j].y - unit.y) < 120)
                        ++score;
                if (score > best) { best = score; target = &unit; }
            }
            if (target && engine.select(hero->x, hero->y)) {
                casting = order(engine, Orders::CastPsionicStorm, target->x, target->y);
                if (casting) { spell_at = frame; previous_energy = hero->energy; }
            }
        }
        if (!resting && (hero->shields < 180 || hero->energy < 75)) {
            resting = true;
            rest_point = {hero->x, hero->y};
        }
        if (resting && hero->shields >= 290 && hero->energy >= 220) resting = false;
        if (frame % 120) continue;
        const UnitView* escort = nullptr;
        int escort_range = 0x7fffffff;
        for (size_t i = 0; i < count; ++i) {
            const auto& unit = units[i];
            if (unit.owner || unit.type >= first_building || unit.type == tassadar ||
                &unit == reserve || (decoy_at && unit.type == marine)) continue;
            const int distance = distance_squared(unit, *hero);
            if (distance < escort_range) { escort_range = distance; escort = &unit; }
            if (engine.select(unit.x, unit.y))
                engine.command(Command::attack, resting ? rest_point.x : goal.x,
                               resting ? rest_point.y : goal.y);
        }
        if (frame <= decoy_at + 36 || casting || frame <= spell_at + 72 ||
            !engine.select(hero->x, hero->y)) continue;
        const bool waiting_for_decoys = stage == 4 && reserve && (!ambush_at || frame - ambush_at < 1650) &&
            hero->y > 3650 && hero->x > 1040 && (!decoy_at || frame - decoy_at < 2500);
        if (resting || (infested && infested_range < 370 * 370) || waiting_for_decoys) {
            order(engine, Orders::HoldPosition);
        } else if (escort && escort_range > 144 * 144) {
            const int dx = hero->x - escort->x, dy = hero->y - escort->y;
            const int span = std::max(1, std::max(std::abs(dx), std::abs(dy)));
            engine.move_selected(escort->x + dx * 128 / span, escort->y + dy * 128 / span);
        } else if (!infested || infested_range > 370 * 370) {
            engine.move_selected(goal.x, goal.y);
        }
    }
    const auto state = engine.snapshot();
    if (defeat) {
        assert(ambush_at && state.winner == 1 && engine.count_units(0, tassadar) == 0);
        printf("Into the Darkness: authored Tassadar-loss defeat at frame %u PASS\n", engine.frame());
        return 0;
    }
    assert(ambush_at && stage == 5 && state.winner == 0);
    assert(engine.count_units(0, tassadar) == 1 && engine.count_units(0, zeratul) == 1);
    assert(ResultScreen::next_mission(unsigned(scenario), true, state.next_scenario) ==
           campaign::key_index("protoss07"));
    const std::string save = output + ".sav";
    assert(engine.save_game(save.c_str(), SaveView{}));
    Engine replay;
    SaveView restored;
    assert(replay.load_save(argv[1], save.c_str(), (output + "-replay").c_str(),
                            (output + "-map").c_str(), restored));
    assert(replay.state_digest() == engine.state_digest());
    printf("Into the Darkness: original Zeratul rescue, exact replay at frame %u and Homeland route PASS\n",
           engine.frame());
}
