// Homeland and The Trial of Tassadar use paid defenses and a Scout fleet.
// Uses public controls and native restrictions, resources and placement only.
// Fog is disabled for repeatable test navigation; no map/triggers/unit stats edited.
#include "bwenums.h"
#include "engine.h"
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <vector>
using namespace port3ds;
int main(int argc, char **argv) {
    assert(argc == 4);
    std::string key = argv[3];
    assert(key == "protoss07" || key == "protoss08");
    const bool trial = key == "protoss08";
    Engine e;
    assert(e.load(argv[1], argv[2],
                  trial ? Scenario::protoss_campaign_8 : Scenario::protoss_campaign_7));
    assert(e.start_recording((std::string("/tmp/sc3ds-original-") + key).c_str()));
    e.set_fog_enabled(false);
    const unsigned worker = 64, base = 154, supply = 156, gas = 157;
    unsigned build_pending = 0, build_at = 0, build_count = 0;
    unsigned cast_after = 0;
    bool attack = false;
    int home_x = 0, home_y = 0;
    for (unsigned frame = 0; frame < 240000 && e.snapshot().winner < 0; ++frame) {
        if (!e.tick()) {
            fprintf(stderr, "%s frame%u: %s\n", key.c_str(), e.frame(), e.error());
            return 1;
        }
        SoundEvent events[256];
        e.drain_sounds(events, 256);
        if (frame % 48)
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
                if (u.type == base) {
                    home_x = u.x;
                    home_y = u.y;
                }
                if (u.completed)
                    parents.push_back(u);
            }
            if (u.mineral)
                minerals.push_back(u);
            if (u.owner == 7 || u.owner == (trial ? 3 : 4))
                enemies.push_back(u);
        }
        if (!home_x)
            continue;
        // Native guards defend the colony until a paid army has assembled.
        unsigned army = e.count_units(0, 65) + e.count_units(0, 66) + e.count_units(0, 70);
        if (army >= (trial ? 40u : 24u) || e.count_units(0, 70) >= 16)
            attack = true;
        if (army < (trial ? 12u : 6u))
            attack = false;

        if (attack && frame % 240 == 0 && !enemies.empty()) {
            const UnitView *target = &enemies.front();
            int distance = std::numeric_limits<int>::max();
            for (const auto &u : enemies) {
                int dx = u.x - home_x, dy = u.y - home_y, d = dx * dx + dy * dy;
                if (d < distance) {
                    distance = d;
                    target = &u;
                }
            }
            for (const auto &u : enemies)
                if (u.type >= 106 && u.type < 202) {
                    int dx = u.x - home_x, dy = u.y - home_y, d = dx * dx + dy * dy;
                    if (target->type < 106 || d < distance) {
                        distance = d;
                        target = &u;
                    }
                }
            for (unsigned i = 0; i < n; ++i)
                if (list[i].owner == 0 &&
                    (list[i].type == 65 || list[i].type == 66 || list[i].type == 70))
                    if (e.select(list[i].x, list[i].y))
                        e.command(Command::attack, target->x, target->y);
        }
        for (const auto &w : workers)
            if (w.order == unsigned(bwgame::Orders::PlayerGuard) ||
                w.order == unsigned(bwgame::Orders::Guard) ||
                w.order == unsigned(bwgame::Orders::Stop)) {
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
        if (build_pending &&
            (e.count_units(0, build_pending) > build_count || frame > build_at + 1500))
            build_pending = 0;
        unsigned want = 0;
        int anchor_x = home_x, anchor_y = home_y;
        for (const auto &p : parents)
            if (p.type == 160 && p.disabled) {
                want = supply;
                anchor_x = p.x;
                anchor_y = p.y;
                break;
            }
        if (!want && state.supply_max - state.supply < 5)
            want = supply;
        if (!want && !e.count_units(0, 160))
            want = 160;
        if (!want && e.count_units(0, 156, false) < 7)
            want = 156;
        if (!want && e.count_units(0, 162, false) < 4)
            want = 162;
        if (want == 156 || want == 162) {
            anchor_x = home_x + 320;
            anchor_y = home_y + 320;
        }
        if (!want && !e.count_units(0, gas))
            want = gas;
        if (!want && key == "protoss07" && e.count_units(0, 160, false) < 4)
            want = 160;
        if (!want && !e.count_units(0, 164))
            want = 164;
        if (!want && e.count_units(0, 167, false) < (trial ? 3u : 2u))
            want = 167;
        if (!build_pending && want && !workers.empty()) {
            bool placed = false;
            auto w = workers.back();
            if (e.select(w.x, w.y)) {
                if (want == gas) {
                    for (unsigned i = 0; i < n && !placed; ++i)
                        if (list[i].geyser && e.placement_unit(want, list[i].x, list[i].y).valid)
                            placed = e.perform(option_code(OptionKind::build, want), list[i].x,
                                               list[i].y);
                } else
                    for (int ring = 2; ring < 11 && !placed; ++ring)
                        for (int y = -ring; y <= ring && !placed; ++y)
                            for (int x = -ring; x <= ring && !placed; ++x) {
                                if (std::abs(x) != ring && std::abs(y) != ring)
                                    continue;
                                int px = anchor_x + x * 32, py = anchor_y + y * 32;
                                if (e.placement_unit(want, px, py).valid)
                                    placed =
                                        e.perform(option_code(OptionKind::build, want), px, py);
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
            else if (p.type == 167 && !e.snapshot().queue)
                e.perform(option_code(OptionKind::train, 70));
            else if (p.type == 160 && (trial ? e.count_units(0, 65) + e.count_units(0, 66) < 12 &&
                                                   state.minerals > 500
                                             : e.count_units(0, 65) < 64 && state.minerals > 250)) {
                unsigned next =
                    trial && e.count_units(0, 164) && e.count_units(0, 66) < 30 ? 66 : 65;
                if (!e.perform(option_code(OptionKind::train, next)) && next == 66)
                    e.perform(option_code(OptionKind::train, 65));
            }
        }
        if (frame >= cast_after)
            for (const auto &caster : parents)
                if (caster.type == 67 && caster.energy >= 75 &&
                    caster.order != unsigned(bwgame::Orders::CastPsionicStorm)) {
                    unsigned score = 0;
                    const UnitView *target = nullptr;
                    for (const auto &enemy : enemies) {
                        int dx = enemy.x - caster.x, dy = enemy.y - caster.y;
                        if (dx * dx + dy * dy > 320 * 320)
                            continue;
                        unsigned count = 0;
                        for (const auto &nearby : enemies) {
                            int x = nearby.x - enemy.x, y = nearby.y - enemy.y;
                            if (x * x + y * y < 72 * 72)
                                ++count;
                        }
                        if (count > score) {
                            score = count;
                            target = &enemy;
                        }
                    }
                    if (target && score >= 2 && e.select(caster.x, caster.y) &&
                        e.perform(option_code(OptionKind::order,
                                              unsigned(bwgame::Orders::CastPsionicStorm)),
                                  target->x, target->y)) {
                        cast_after = frame + 96;
                        break;
                    }
                }
        if (frame % 4800 == 0) {
            printf("Scouts%u Stargates%u enemies%zu ", e.count_units(0, 70), e.count_units(0, 167),
                   enemies.size());
            printf(
                "%s frame%u minerals%d gas%d workers%u army%u buildings%u/%u pending%u attack%d\n",
                key.c_str(), e.frame(), state.minerals, state.gas, e.count_units(0, worker), army,
                e.count_units(0, 160), e.count_units(0, 164), build_pending, attack);
            fflush(stdout);
        }
    }
    auto s = e.snapshot();
    printf("%s result%d frame%u\n", key.c_str(), s.winner, e.frame());
    fflush(stdout);
    if (s.winner != 0)
        return 1;
    SaveView view;
    assert(
        e.save_game((std::string("/tmp/sc3ds-") + key.c_str() + "-completed.sav").c_str(), view));
    Engine replay;
    SaveView restored;
    assert(replay.load_save(
        argv[1], (std::string("/tmp/sc3ds-") + key.c_str() + "-completed.sav").c_str(),
        (std::string("/tmp/sc3ds-third-replay-") + key.c_str()).c_str(),
        (std::string("/tmp/sc3ds-third-map-") + key.c_str()).c_str(), restored));
    assert(e.state_digest() == replay.state_digest());
    puts("Original late Protoss paid fleet victory and exact replay PASS");
}
