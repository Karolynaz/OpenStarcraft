// Original Choosing Sides, public controls only. Fog-off navigation aid.
#include "bwenums.h"
#include "engine.h"
#include <cassert>
#include <cstdio>
#include <vector>
using namespace port3ds;
using namespace bwgame;
struct Point {
    int x, y;
};

int main(int argc, char **argv) {
    assert(argc == 3);
    Engine e;
    assert(e.load(argv[1], argv[2], Scenario::protoss_campaign_5));
    assert(e.start_recording("/tmp/sc3ds-protoss05-journal"));
    e.set_fog_enabled(false);
    bool loaded = false, unloaded = false, active = false;
    unsigned stage = 0, pending = 0, pending_at = 0;
    std::vector<Point> route;
    for (unsigned f = 0; f < 80000 && e.snapshot().winner < 0; ++f) {
        if (!e.tick()) {
            printf("ERROR %s\n", e.error());
            return 1;
        }
        SoundEvent snd[256];
        e.drain_sounds(snd, 256);
        if (f % 24)
            continue;
        UnitView u[1700];
        unsigned n = e.visible_units(u, 1700);
        UnitView *ship = nullptr;
        UnitView *hero = nullptr;
        std::vector<UnitView> zealots;
        for (unsigned i = 0; i < n; ++i)
            if (u[i].owner == 0) {
                if (u[i].type == 69 && (!ship || u[i].cargo > ship->cargo))
                    ship = &u[i];
                if (u[i].type == 79)
                    hero = &u[i];
                if (u[i].type == 65)
                    zealots.push_back(u[i]);
            }
        if (!loaded) {
            auto state = e.snapshot();
            if (pending && (e.count_units(0, pending, false) || f > pending_at + 1800))
                pending = 0;
            unsigned want = !e.count_units(0, 157, false)         ? 157
                            : !e.count_units(0, 164, false)       ? 164
                            : !e.count_units(0, 155, false)       ? 155
                            : !e.count_units(0, 171, false)       ? 171
                            : state.supply_max - state.supply < 4 ? 156
                                                                  : 0;
            for (unsigned i = 0; i < n; ++i)
                if (!u[i].owner && u[i].type == 64) {
                    if (want && !pending && e.select(u[i].x, u[i].y)) {
                        bool built = false;
                        if (want == 157) {
                            for (unsigned j = 0; j < n && !built; ++j)
                                if (u[j].geyser && abs(u[j].x - 736) + abs(u[j].y - 1392) < 900 &&
                                    e.placement_unit(want, u[j].x, u[j].y).valid)
                                    built = e.perform(option_code(OptionKind::build, want), u[j].x,
                                                      u[j].y);
                        } else
                            for (int r = 2; r < 12 && !built; ++r)
                                for (int dy = -r; dy <= r && !built; ++dy)
                                    for (int dx = -r; dx <= r && !built; ++dx) {
                                        int x = 736 + dx * 32, y = 1392 + dy * 32;
                                        if (e.placement_unit(want, x, y).valid)
                                            built = e.perform(option_code(OptionKind::build, want),
                                                              x, y);
                                    }
                        if (built) {
                            pending = want;
                            pending_at = f;
                        }
                    }
                    if (u[i].order == 1 || u[i].order == 2 || u[i].order == 3) {
                        const UnitView *t = nullptr;
                        int best = 0x7fffffff;
                        for (unsigned j = 0; j < n; ++j)
                            if (u[j].mineral) {
                                int dx = u[j].x - u[i].x, dy = u[j].y - u[i].y,
                                    d = dx * dx + dy * dy;
                                if (d < best) {
                                    best = d;
                                    t = &u[j];
                                }
                            }
                        if (t && e.select(u[i].x, u[i].y))
                            e.command(Command::context, t->x, t->y);
                    }
                }
            if (f % 480 == 0) {
                unsigned assigned = 0;
                for (unsigned i = 0; i < n; ++i)
                    if (!u[i].owner && u[i].type == 157 && u[i].completed)
                        for (unsigned j = 0; j < n && assigned < 3; ++j)
                            if (!u[j].owner && u[j].type == 64 && e.select(u[j].x, u[j].y)) {
                                if (e.command(Command::context, u[i].x + 40, u[i].y + 16))
                                    ++assigned;
                            }
            }
            for (unsigned i = 0; i < n; ++i)
                if (!u[i].owner && u[i].completed && e.select(u[i].x, u[i].y) &&
                    !e.snapshot().queue) {
                    if (u[i].type == 154 && e.count_units(0, 64) < 12)
                        e.perform(option_code(OptionKind::train, 64));
                    if (u[i].type == 155 && e.count_units(0, 69, false) < 8)
                        e.perform(option_code(OptionKind::train, 69));
                    if (u[i].type == 171 &&
                        !e.upgrade_level(unsigned(UpgradeTypes::Gravitic_Drive)))
                        e.perform(option_code(OptionKind::upgrade,
                                              unsigned(UpgradeTypes::Gravitic_Drive)));
                }
        }
        if (!loaded && ship) {
            if (ship->cargo >= 3 && e.count_units(0, 69) >= 8 &&
                e.upgrade_level(unsigned(UpgradeTypes::Gravitic_Drive))) {
                loaded = true;
                route = {{32, 32}, {4064, 32}, {4064, 4032}, {3504, 4032}, {3504, 3456}};
                printf("Loaded shuttle f%u path%zu\n", e.frame(), route.size());
            } else if (ship->cargo < 3) {
                const UnitView *passenger = hero             ? hero
                                            : zealots.size() ? &zealots.front()
                                                             : nullptr;
                if (passenger && e.select(ship->x, ship->y))
                    e.perform(option_code(OptionKind::order, unsigned(Orders::PickupTransport)),
                              passenger->x, passenger->y);
            }
        }
        if (loaded && ship && !unloaded) {
            auto p = route[stage];
            int dx = ship->x - p.x, dy = ship->y - p.y;
            if (dx * dx + dy * dy < 64 * 64) {
                if (stage + 1 < route.size())
                    ++stage;
                else if (e.select(ship->x, ship->y))
                    unloaded = e.perform(option_code(OptionKind::unload));
            }
            if (!unloaded)
                for (unsigned i = 0; i < n; ++i)
                    if (!u[i].owner && u[i].type == 69 && e.select(u[i].x, u[i].y))
                        e.move_selected(route[stage].x, route[stage].y);
        }
        if (unloaded && f % 120 == 0)
            for (unsigned i = 0; i < n; ++i)
                if (u[i].owner == 0 && (u[i].type == 79 || u[i].type == 65) &&
                    e.select(u[i].x, u[i].y))
                    e.command(Command::attack, 3504, 3456);
        if (loaded && !active) {
            SaveView view;
            assert(e.save_game("/tmp/sc3ds-protoss05-active.sav", view));
            Engine r;
            SaveView v;
            assert(r.load_save(argv[1], "/tmp/sc3ds-protoss05-active.sav",
                               "/tmp/sc3ds-protoss05-active-j", "/tmp/sc3ds-protoss05-active-m",
                               v));
            assert(r.state_digest() == e.state_digest());
            active = true;
        }
        if (f % 2400 == 0) {
            printf("P5 f%u ship%d,%d hp%d cargo%u hero%d zealots%zu stage%u unloaded%d\n",
                   e.frame(), ship ? ship->x : 0, ship ? ship->y : 0, ship ? ship->hp : 0,
                   ship ? ship->cargo : 0, hero ? hero->hp : 0, zealots.size(), stage, unloaded);
            fflush(stdout);
        }
    }
    printf("P5 result%d f%u\n", e.snapshot().winner, e.frame());
    assert(e.snapshot().winner == 0 && loaded && active);
    SaveView view;
    assert(e.save_game("/tmp/sc3ds-protoss05-completed.sav", view));
    Engine replay;
    SaveView restored;
    assert(replay.load_save(argv[1], "/tmp/sc3ds-protoss05-completed.sav",
                            "/tmp/sc3ds-protoss05-replay", "/tmp/sc3ds-protoss05-map", restored));
    assert(e.state_digest() == replay.state_digest());
    puts("Choosing Sides original transport / Tassadar and Zealots / original victory / active and "
         "completed exact replay PASS");
}
