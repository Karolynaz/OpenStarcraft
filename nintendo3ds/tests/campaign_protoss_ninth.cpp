// Shadow Hunters on the original CHK. Exercise the authored Cerebrate and hero conditions.
#include "bwenums.h"
#include "engine.h"
#include "result_screen.h"
#include <algorithm>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <vector>
using namespace port3ds;
using namespace bwgame;
static unsigned type(UnitTypes unit) { return unsigned(unit); }
struct Point {
    int x, y;
};
int main(int argc, char **argv) {
    assert(argc == 3);
    const auto scenario = Scenario::protoss_campaign_9;
    Engine e;
    assert(e.load(argv[1], argv[2], scenario));
    assert(e.start_recording("/tmp/sc3ds-shadow-hunters-session"));
    // The pilot uses the whole map to choose orders. The mission keeps its default fog setting.
    e.set_fog_enabled(false);
    const auto worker = type(UnitTypes::Protoss_Probe), base = type(UnitTypes::Protoss_Nexus);
    const auto supply = type(UnitTypes::Protoss_Pylon), gas = type(UnitTypes::Protoss_Assimilator);
    const auto zergling = type(UnitTypes::Zerg_Zergling);
    const auto hydralisk = type(UnitTypes::Zerg_Hydralisk);
    const auto ultralisk = type(UnitTypes::Zerg_Ultralisk);
    const auto mutalisk = type(UnitTypes::Zerg_Mutalisk);
    const auto guardian = type(UnitTypes::Zerg_Guardian);
    const auto infested_terran = type(UnitTypes::Zerg_Infested_Terran);
    const auto zealot = type(UnitTypes::Protoss_Zealot);
    const auto dragoon = type(UnitTypes::Protoss_Dragoon);
    const auto arbiter = type(UnitTypes::Protoss_Arbiter);
    const auto zeratul = type(UnitTypes::Hero_Zeratul);
    const auto fenix_dragoon = type(UnitTypes::Hero_Fenix_Dragoon);
    const auto reaver = type(UnitTypes::Protoss_Reaver);
    const auto observer = type(UnitTypes::Protoss_Observer);
    const auto scarab = type(UnitTypes::Protoss_Scarab);
    const auto lurker = type(UnitTypes::Zerg_Lurker);
    const auto sunken_colony = type(UnitTypes::Zerg_Sunken_Colony);
    const auto cerebrate = type(UnitTypes::Special_Cerebrate);
    const auto robotics_facility = type(UnitTypes::Protoss_Robotics_Facility);
    const auto observatory = type(UnitTypes::Protoss_Observatory);
    const auto gateway = type(UnitTypes::Protoss_Gateway);
    const auto photon_cannon = type(UnitTypes::Protoss_Photon_Cannon);
    const auto cybernetics_core = type(UnitTypes::Protoss_Cybernetics_Core);
    const auto forge = type(UnitTypes::Protoss_Forge);
    const auto robotics_support_bay = type(UnitTypes::Protoss_Robotics_Support_Bay);
    const auto resource_mineral_field = type(UnitTypes::Resource_Mineral_Field);
    unsigned build_pending = 0, build_at = 0, build_count = 0;
    int home_x = 0, home_y = 0;
    unsigned wave = 0, leg = 0, last_advance = 0, clear_since = 0, east_start = 0;
    bool launched = false;
    for (unsigned frame = 0; frame < 144000 && e.snapshot().winner < 0; ++frame) {
        if (!e.tick()) {
            fprintf(stderr, "Shadow Hunters frame %u: %s\n", e.frame(), e.error());
            return 1;
        }
        SoundEvent events[256];
        e.drain_sounds(events, 256);
        if (frame % 24)
            continue;
        UnitView list[1700];
        unsigned n = e.visible_units(list, 1700);
        auto state = e.snapshot();
        std::vector<UnitView> workers, parents;
        std::vector<UnitView> minerals;
        std::vector<UnitView> enemies;
        for (unsigned i = 0; i < n; ++i) {
            auto &u = list[i];
            if (u.owner == 0) {
                if (u.type == worker && u.completed)
                    workers.push_back(u);
                if (u.type == base && u.y > 2500) {
                    home_x = u.x;
                    home_y = u.y;
                }
                if (u.completed)
                    parents.push_back(u);
            }
            if (u.mineral)
                minerals.push_back(u);
            if (u.owner == 5 || u.owner == 6 || u.owner == 7)
                enemies.push_back(u);
        }
        if (!home_x && !workers.empty()) {
            if (e.select(workers.front().x, workers.front().y)) {
                bool built = false;
                for (int r = 0; r < 12 && !built; ++r)
                    for (int y = -r; y <= r && !built; ++y)
                        for (int x = -r; x <= r && !built; ++x) {
                            int px = 1120 + x * 32, py = 3712 + y * 32;
                            if (e.placement_unit(base, px, py).valid)
                                built = e.perform(option_code(OptionKind::build, base), px, py);
                        }
            }
            for (const auto &u : parents)
                if (u.type < 106 && !u.worker && e.select(u.x, u.y))
                    e.perform(option_code(OptionKind::order, unsigned(bwgame::Orders::AttackMove)), 1120,
                              3712);
        }
        if (!home_x)
            continue;

        const UnitView *hero = nullptr, *brain = nullptr;
        for (unsigned i = 0; i < n; ++i)
            if (list[i].type == zeratul && !list[i].owner)
                hero = list + i;
        for (const auto &u : enemies)
            if (u.type == cerebrate && u.owner == (wave ? 6 : 5))
                brain = &u;
        unsigned army = e.count_units(0, zealot) + e.count_units(0, dragoon) + e.count_units(0, reaver);
        if (!launched && army >= 20 && e.count_units(0, reaver) >= 3) {
            launched = true;
            last_advance = frame;
            printf("Launched at %u with army %u\n", frame, army);
        }
        const Point west[] = {{1680, 3584}, {1760, 2816}, {1776, 2320}, {1776, 1920},
                              {1632, 1520}, {1360, 1160}, {1328, 880},  {880, 840},
                              {736, 584},   {720, 352},   {720, 192}};
        const Point east[] = {{896, 128},  {1200, 80},  {1480, 72},  {1800, 88}, {2024, 124},
                              {2224, 152}, {2400, 160}, {2512, 176}, {2608, 192}};
        const unsigned count = wave ? sizeof(east) / sizeof(*east) : sizeof(west) / sizeof(*west);
        // Keep the wounded hero on the west side while the ground army reforms.
        const bool regroup = wave && frame < east_start + 5700;
        Point goal = wave ? east[leg] : west[leg];
        if (launched && !regroup && leg + 1 < count) {
            unsigned nearby = 0;
            for (const auto &u : parents)
                if ((u.type == dragoon || u.type == reaver || u.type == zealot) &&
                    abs(u.x - goal.x) + abs(u.y - goal.y) < 288)
                    ++nearby;
            if ((nearby >= 4 || (frame - last_advance > 2400 && nearby >= 1)) &&
                (wave || leg >= 6 || (hero && abs(hero->x - goal.x) + abs(hero->y - goal.y) < 650))) {
                ++leg;
                last_advance = frame;
                goal = wave ? east[leg] : west[leg];
                printf("wave%u leg%u f%u army%u\n", wave, leg, frame, army);
                fflush(stdout);
            }
        }
        if (launched && !brain && wave == 0) {
            wave = 1;
            leg = 0;
            last_advance = frame;
            east_start = frame;
            clear_since = 0;
            printf("West Cerebrate eliminated f%u\n", frame);
            fflush(stdout);
        }

        unsigned guards = 0, escorts = 0;
        const bool assault = launched && leg + 2 >= count && brain;
        auto d = [](const UnitView &a, const UnitView &b) { return abs(a.x - b.x) + abs(a.y - b.y); };
        if (assault)
            for (auto &u : enemies) {
                if (d(u, *brain) > 416)
                    continue;
                if (u.type == zergling || u.type == hydralisk || u.type == ultralisk ||
                    u.type == infested_terran || u.type == lurker || u.type == sunken_colony ||
                    u.type == mutalisk || u.type == guardian)
                    ++guards;
            }
        if (assault)
            for (auto &u : parents)
                if ((u.type == dragoon || u.type == zealot || u.type == reaver) && d(u, *brain) < 400)
                    ++escorts;
        if (assault && guards <= 2 && escorts >= 2) {
            if (!clear_since)
                clear_since = frame;
        } else if (!assault || guards >= 8)
            clear_since = 0;
        if (frame % 120 == 0) {
            const bool near_brain = hero && brain && abs(hero->x - brain->x) + abs(hero->y - brain->y) < 96;
            for (const auto &u : parents)
                if ((u.type == fenix_dragoon || u.type == arbiter || u.type == zeratul ||
                     u.type == zealot || u.type == dragoon || u.type == reaver || u.type == observer) &&
                    e.select(u.x, u.y)) {
                    if (u.type == zeratul) {
                        if (!launched) {
                            if (abs(u.x - home_x) + abs(u.y - home_y) > 256)
                                e.move_selected(home_x, home_y);
                        } else if (wave && (!assault || !clear_since || u.shields < 180)) {
                            e.move_selected(736, 584);
                        } else if (assault) {
                            const auto safe = wave ? east[count - 3] : west[count - 4];
                            if (clear_since && frame > unsigned(clear_since) + 48 && u.shields >= 150) {
                                if (wave && u.x < 1770)
                                    e.move_selected(1840, 114);
                                else
                                    e.command(Command::context, brain->x, brain->y);
                            } else
                                e.move_selected(safe.x, safe.y);
                        } else if (leg >= 6) {
                            const auto p = wave ? east[leg - 1] : west[leg - 1];
                            e.command(Command::attack, p.x, p.y);
                        } else
                            e.command(Command::attack, goal.x, goal.y);
                    } else if (!launched || u.type == arbiter || (u.type == fenix_dragoon && !wave)) {
                        if (abs(u.x - home_x) + abs(u.y - home_y) > 500)
                            e.command(Command::attack, home_x + 80, home_y + 20);
                    } else if (u.type == observer) {
                        const auto p = wave ? east[leg ? leg - 1 : 0] : west[leg ? leg - 1 : 0];
                        e.move_selected(p.x, p.y);
                    } else if (regroup) {
                        e.command(Command::attack, 900, 780);
                    } else if (assault) {
                        const UnitView *target = nullptr;
                        int best = 99999;
                        for (auto &v : enemies) {
                            if (d(v, *brain) > 550 || v.flying && u.type != dragoon ||
                                v.type == cerebrate || v.type >= resource_mineral_field)
                                continue;
                            int score = d(u, v);
                            if (v.type == sunken_colony || v.type == hydralisk)
                                score -= 45;
                            if (score < best) {
                                best = score;
                                target = &v;
                            }
                        }
                        if (near_brain && !guards && d(u, *brain) < 430)
                            e.command(Command::context, brain->x, brain->y);
                        else if (target && d(*target, u) < 550)
                            e.command(Command::context, target->x, target->y);
                        else {
                            const auto p = wave ? east[count - 2] : west[count - 2];
                            e.command(Command::attack, p.x, p.y);
                        }
                    } else
                        e.command(Command::attack, goal.x, goal.y);
                }
        }
        for (const auto &w : workers)
            if (w.order == unsigned(bwgame::Orders::PlayerGuard) ||
                w.order == unsigned(bwgame::Orders::Guard) || w.order == unsigned(bwgame::Orders::Stop)) {
                const UnitView *target = nullptr;
                int distance = std::numeric_limits<int>::max();
                for (const auto &u : minerals) {
                    int dx = u.x - w.x, dy = u.y - w.y, d = dx * dx + dy * dy;
                    if (d < distance) {
                        distance = d;
                        target = &u;
                    }
                }
                if (target && e.select(w.x, w.y))
                    e.command(Command::context, target->x, target->y);
            }
        // Assign three workers to the paid refinery once it has completed.
        if (frame % 480 == 0 && e.count_units(0, gas) && workers.size() >= 7) {
            UnitView *refinery = nullptr;
            for (unsigned i = 0; i < n; ++i)
                if (list[i].owner == 0 && list[i].type == gas && list[i].completed)
                    refinery = &list[i];
            if (refinery)
                for (unsigned i = 0; i < 3; ++i)
                    if (e.select(workers[i].x, workers[i].y))
                        e.command(Command::context, refinery->x + 40, refinery->y + 16);
        }
        if (build_pending && (e.count_units(0, build_pending) > build_count || frame > build_at + 1500))
            build_pending = 0;
        unsigned want = 0;
        int anchor_x = home_x, anchor_y = home_y;
        for (const auto &p : parents)
            if (p.type == gateway && p.disabled) {
                want = supply;
                anchor_x = p.x;
                anchor_y = p.y;
                break;
            }
        if (!want && state.supply_max - state.supply < 5)
            want = supply;
        if (!want && !e.count_units(0, gateway))
            want = gateway;
        if (!want && e.count_units(0, supply, false) < 2)
            want = supply;
        if (!want && !e.count_units(0, forge, false))
            want = forge;
        if (!want && e.count_units(0, photon_cannon, false) < 2)
            want = photon_cannon;
        if (want == supply || want == photon_cannon) {
            anchor_x = home_x - 192;
            anchor_y = home_y - 160;
        }
        if (!want && !e.count_units(0, gas, false))
            want = gas;

        if (!want && !e.count_units(0, cybernetics_core, false))
            want = cybernetics_core;
        if (!want && e.count_units(0, robotics_facility, false) < 1)
            want = robotics_facility;
        if (!want && !e.count_units(0, robotics_support_bay, false))
            want = robotics_support_bay;
        if (!want && !e.count_units(0, observatory, false))
            want = observatory;
        if (!want && e.count_units(0, gateway, false) < (wave ? 3u : 2u))
            want = gateway;
        if (wave && e.count_units(0, base, false) < 2 && state.minerals >= 400) {
            want = base;
            anchor_x = 1112;
            anchor_y = 170;
        }
        if (!build_pending && want && !workers.empty()) {
            bool placed = false;
            auto w = workers.back();
            if (e.select(w.x, w.y)) {
                if (want == gas) {
                    for (unsigned i = 0; i < n && !placed; ++i)
                        if (list[i].geyser && abs(list[i].x - home_x) + abs(list[i].y - home_y) < 2400 &&
                            e.placement_unit(want, list[i].x, list[i].y).valid)
                            placed = e.perform(option_code(OptionKind::build, want), list[i].x, list[i].y);
                } else
                    for (int ring = 2; ring < 11 && !placed; ++ring)
                        for (int y = -ring; y <= ring && !placed; ++y)
                            for (int x = -ring; x <= ring && !placed; ++x) {
                                if (std::abs(x) != ring && std::abs(y) != ring)
                                    continue;
                                int px = anchor_x + x * 32, py = anchor_y + y * 32;
                                if (e.placement_unit(want, px, py).valid)
                                    placed = e.perform(option_code(OptionKind::build, want), px, py);
                            }
            }
            if (placed) {
                build_pending = want;
                build_at = frame;
                build_count = e.count_units(0, want);
            }
        }
        for (const auto &p : parents) {
            if (!e.select(p.x, p.y))
                continue;
            if (p.type == base && e.count_units(0, worker) < 22)
                e.perform(option_code(OptionKind::train, worker));
            else if (p.type == robotics_facility && e.count_units(0, observatory) &&
                     !e.count_units(0, observer, false) && !e.snapshot().queue)
                e.perform(option_code(OptionKind::train, observer));
            else if (p.type == robotics_facility && !e.snapshot().queue &&
                     e.count_units(0, reaver, false) < 6)
                e.perform(option_code(OptionKind::train, reaver));
            else if (p.type == reaver && p.fighters < 5 && !e.snapshot().queue)
                e.perform(option_code(OptionKind::train, scarab));
            else if (p.type == gateway && e.count_units(0, zealot) + e.count_units(0, dragoon) < 22 &&
                     state.minerals > 300 && !e.snapshot().queue) {
                unsigned next = e.count_units(0, dragoon) < 20 ? dragoon : zealot;
                if (!e.perform(option_code(OptionKind::train, next)) && next == dragoon)
                    e.perform(option_code(OptionKind::train, zealot));
            }
        }
    }
    const auto state = e.snapshot();
    assert(launched && wave == 1 && state.winner == 0);
    assert(e.count_units(0, type(UnitTypes::Hero_Zeratul)) == 1);
    assert(e.count_units(0, type(UnitTypes::Hero_Fenix_Dragoon)) == 1);
    assert(e.count_units(5, type(UnitTypes::Special_Cerebrate)) == 0);
    assert(e.count_units(6, type(UnitTypes::Special_Cerebrate)) == 0);
    const auto next = ResultScreen::next_mission(unsigned(scenario), true, state.next_scenario);
    assert(next == campaign::key_index("protoss10"));
    const char *path = "/tmp/sc3ds-protoss09-completed.sav";
    assert(e.save_game(path, SaveView{}));
    Engine replay;
    SaveView restored;
    assert(replay.load_save(argv[1], path, "/tmp/sc3ds-shadow-hunters-replay",
                            "/tmp/sc3ds-shadow-hunters-map", restored));
    assert(replay.state_digest() == e.state_digest());
    printf("Shadow Hunters: both Cerebrates defeated, both heroes alive, exact replay at frame %u and Eye "
           "of the Storm route PASS\n",
           e.frame());
}
